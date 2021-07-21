#include "traj_min_jerk.hpp"
#include "traj_min_snap.hpp"

#include <ros/ros.h>

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <Eigen/Eigen>

#include <fstream>
// #include <string>
// #include <sstream>
#include <stdlib.h>

using namespace std;
using namespace Eigen;

/*
// get waypoints from file.
*/
class WayPointsReader
{
    private:
        ifstream infile;

    public:
        int N;

        WayPointsReader();
        MatrixXd read();
};

WayPointsReader::WayPointsReader()
{
    infile.open("/home/johanna/catkin_ws/src/large_scale_traj_optimizer/src/waypoints.txt");
}

MatrixXd WayPointsReader::read()
{
    infile>>N;

    MatrixXd waypoints(3, N);
    Array3d tmp;

    waypoints.col(0).setZero();

    int i = 0;
    double x, y, z;
    while (infile>>x>>y>>z)
    {
        // printf("waypoint %d: %.2f, %.2f, %.2f\n",i,x,y,z);
        if (i >= N)
        {
            printf("More waypoints than setting!\n");
            printf("Using only first %d waypoints.\n", N);
            break;
        }

        // give in waypoint (x, y, z)
        tmp << x, y, z;
        waypoints.col(i) << tmp;

        i++;
    }
    if (i < N)
    {
        printf("Less waypoints than setting!\n");
        printf("Setting last %d wapoints as origin point.\n", N-i);
    }

    return waypoints;
}

/*
// write trajectory in file.
// use fprintf to save time.
*/
class TrajectoryWriter
{
    private:
        FILE *fp_pos;
        FILE *fp_vel;
        FILE *fp_acc;

    public:
        TrajectoryWriter();
        void writePos (Vector3d p);
        void writeVel (Vector3d v);
        void writeAcc (Vector3d a);
};

TrajectoryWriter::TrajectoryWriter()
{
    fp_pos = fopen("/home/johanna/catkin_ws/src/large_scale_traj_optimizer/src/traj_p.txt", "w+");
    fp_vel = fopen("/home/johanna/catkin_ws/src/large_scale_traj_optimizer/src/traj_v.txt", "w+");
    fp_acc = fopen("/home/johanna/catkin_ws/src/large_scale_traj_optimizer/src/traj_a.txt", "w+");
}

void TrajectoryWriter::writePos (Vector3d p)
{
    fprintf(fp_pos, "%.4f %.4f %.4f\n", p(0), p(1), p(2));
}
void TrajectoryWriter::writeVel (Vector3d v)
{
    fprintf(fp_vel, "%.4f %.4f %.4f\n", v(0), v(1), v(2));
}
void TrajectoryWriter::writeAcc (Vector3d a)
{
    fprintf(fp_acc, "%.4f %.4f %.4f\n", a(0), a(1), a(2));
}

/*
// allocate time for waypoints
*/
VectorXd allocateTime(const MatrixXd &wayPs,
                      double vel,
                      double acc)
{
    int N = (int)(wayPs.cols()) - 1;
    VectorXd durations(N);
    if (N > 0)
    {
        Eigen::Vector3d p0, p1;
        double dtxyz, D, acct, accd, dcct, dccd, t1, t2, t3;

        for (int k = 0; k < N; k++)
        {
            p0 = wayPs.col(k);
            p1 = wayPs.col(k + 1);
            D = (p1 - p0).norm();       // distance

            acct = vel / acc;                   // accelerate time
            accd = (acc * acct * acct / 2);     // accelerate distance
            dcct = vel / acc;                   // de-accelerate time
            dccd = acc * dcct * dcct / 2;       // de-accelerate distance

            if (D < accd + dccd)
            {
                t1 = sqrt(acc * D) / acc;
                t2 = (acc * t1) / acc;
                dtxyz = t1 + t2;
            }
            else
            {
                t1 = acct;
                t2 = (D - accd - dccd) / vel;
                t3 = dcct;
                dtxyz = t1 + t2 + t3;
            }

            durations(k) = dtxyz;
        }
    }

    return durations;
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "traj_node");
    ros::NodeHandle nh_;
    ros::Rate lp(1000);

    min_jerk::JerkOpt jerkOpt;              // optimizer
    min_jerk::Trajectory minJerkTraj;       // trajectory generated by optimizer

    // read waypoints from file
    WayPointsReader waypGen;
    MatrixXd waypoints;    // pva * time
    waypoints = waypGen.read(); 
    // waypGen.read(waypoints);

    // allocate time for waypoints
    VectorXd ts(waypGen.N);
    double MaxVel = 15.0, MaxVelCal = 15.0;
    ts = allocateTime(waypoints, MaxVelCal, 5.0);

    // intial & finial state
    Matrix3d iS, fS;        // xyz * pva
    iS.setZero();
    fS.setZero();
    iS.col(0) << waypoints.leftCols<1>();
    fS.col(0) << waypoints.rightCols<1>();

    // set initial & finial waypoints
    jerkOpt.reset(iS, fS, waypGen.N - 1);
    // generate trajectory
    jerkOpt.generate(waypoints.block(0, 1, 3, waypGen.N - 2), ts);
    // get trajectory into `minJerkTraj`
    jerkOpt.getTraj(minJerkTraj);

    // check maximum velocity
    while (!minJerkTraj.checkMaxVelRate(MaxVel))
    {
        printf("maximum velocity")
        MaxVelCal = MaxVelCal - 0.5;
        ts = allocateTime(waypoints, MaxVelCal, 5.0);
        jerkOpt.generate(waypoints.block(0, 1, 3, waypGen.N - 2), ts);
        jerkOpt.getTraj(minJerkTraj);
    }

    // write trajectory
    TrajectoryWriter trajout;
    int frq = 100;
    double endtime = 0.0;
    for (int i = 0; i < waypGen.N - 1;i++)
        endtime += ts(i);
    int steps = floor(frq * endtime);
    for (int i = 0; i < steps; i++)
    {
        double timee = 1.0 / frq * i;
        // printf("%.2f\n",timee);
        Vector3d p,v,a;
        trajout.writePos(minJerkTraj.getPos(timee));
        trajout.writeVel(minJerkTraj.getVel(timee));
        trajout.writeAcc(minJerkTraj.getAcc(timee));
    }

    ros::spinOnce();
    lp.sleep();

    return 0;
}

