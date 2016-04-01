/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "SteeringWheelTask.hpp"
#include <controldev/LogitechG27.hpp>
#include <rtt/extras/FileDescriptorActivity.hpp>
#include <unistd.h>
#include <fcntl.h>

using namespace controldev;

SteeringWheelTask::SteeringWheelTask(std::string const& name)
    : SteeringWheelTaskBase(name), steerControl(new controldev::LogitechG27())
{
}

SteeringWheelTask::SteeringWheelTask(std::string const& name, RTT::ExecutionEngine* engine)
    : SteeringWheelTaskBase(name, engine), steerControl(new controldev::LogitechG27())
{
}

SteeringWheelTask::~SteeringWheelTask()
{
    delete steerControl;
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See SteeringWheelTask.hpp for more detailed
// documentation about them.

bool SteeringWheelTask::configureHook()
{
    if (! SteeringWheelTaskBase::configureHook())
        return false;
    
    if (!steerControl->init())
    {
        std::cerr << "Warning: Unable to open G27 Racing Wheel device " << std::endl;
	return false;
    }

    return true;
}

bool SteeringWheelTask::startHook()
{
    if (! SteeringWheelTaskBase::startHook())
        return false;
    
    RTT::extras::FileDescriptorActivity* activity =
        getActivity<RTT::extras::FileDescriptorActivity>();
    if (activity)
    {
	activity->watch(steerControl->getFileDescriptor());
    }
    
    return true;
}

void SteeringWheelTask::updateHook()
{
    SteeringWheelTaskBase::updateHook();
    
    base::commands::Motion2D mcmd;
    
    RawCommand rcmd;
    
    bool update = false;

    while(steerControl->updateState())
    {
	update = true;
    }
    
    if(!update)
	return;

    rcmd.deviceIdentifier = steerControl->getName();

    rcmd.axes.elements.resize(5);

    rcmd.axes.elements[1] = steerControl->getAxis(LogitechG27::AXIS_Wheel);
    rcmd.axes.elements[0] = steerControl->getAxis(LogitechG27::AXIS_Clutchdirupdown);
    rcmd.axes.elements[2] = steerControl->getAxis(LogitechG27::AXIS_Clutchdirleftright); 
    rcmd.axes.elements[3] = steerControl->getAxis(LogitechG27::AXIS_Throttle);
    rcmd.axes.elements[4] = steerControl->getAxis(LogitechG27::AXIS_Brake);

    float max_speed = _maxSpeed.get();
    float min_speed = _minSpeed.get();
    float max_speed_ratio = (rcmd.axes.elements[3] + min_speed) / (1.0 + min_speed);
    float max_rotation_speed = _maxRotationSpeed.get();
    double x = rcmd.axes.elements[3]  * max_speed;
    double y = rcmd.axes.elements[1];

    mcmd.rotation = y;
    mcmd.translation = x;
    
    // Send motion command
    _motion_command.write(mcmd);

    int buttonCount = steerControl->getNrButtons();
    
    // Set button bit list
    for (int i = 0; i < buttonCount; i++)
    {
        rcmd.buttons.elements.push_back(steerControl->getButtonPressed(i));
    }
    
    // Send raw command
    _raw_command.write(rcmd);
}

void SteeringWheelTask::stopHook()
{
    RTT::extras::FileDescriptorActivity* activity =
        getActivity<RTT::extras::FileDescriptorActivity>();
    activity->clearAllWatches();

    SteeringWheelTaskBase::stopHook();
}
