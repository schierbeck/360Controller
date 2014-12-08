/*
 MICE Xbox 360 Controller driver for Mac OS X
 Copyright (C) 2006-2013 Colin Munro
 
 Controller.cpp - driver class for the 360 controller
 
 This file is part of Xbox360Controller.
 
 Xbox360Controller is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 Xbox360Controller is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with Foobar; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <IOKit/usb/IOUSBDevice.h>
#include <IOKit/usb/IOUSBInterface.h>
#include "Controller.h"
#include "ControlStruct.h"
namespace HID_360 {
#include "xbox360hid.h"
}
#include "_60Controller.h"

OSDefineMetaClassAndStructors(Xbox360ControllerClass, IOHIDDevice)

static Xbox360Peripheral* GetOwner(const IOService *us)
{
	IOService *prov = us->getProvider();
    
	if (prov == NULL)
		return NULL;
	return OSDynamicCast(Xbox360Peripheral, prov);
}

bool Xbox360ControllerClass::start(IOService *provider)
{
    if (OSDynamicCast(Xbox360Peripheral, provider) == NULL)
        return false;
    return IOHIDDevice::start(provider);
}

IOReturn Xbox360ControllerClass::setProperties(OSObject *properties)
{
	Xbox360Peripheral *owner = GetOwner(this);
	if (owner == NULL)
		return kIOReturnUnsupported;
	return owner->setProperties(properties);
}

// Returns the HID descriptor for this device
IOReturn Xbox360ControllerClass::newReportDescriptor(IOMemoryDescriptor **descriptor) const
{
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task,0,sizeof(HID_360::ReportDescriptor));
    
    if (buffer == NULL) return kIOReturnNoResources;
    buffer->writeBytes(0,HID_360::ReportDescriptor,sizeof(HID_360::ReportDescriptor));
    *descriptor=buffer;
    return kIOReturnSuccess;
}

// Handles a message from the userspace IOHIDDeviceInterface122::setReport function
IOReturn Xbox360ControllerClass::setReport(IOMemoryDescriptor *report,IOHIDReportType reportType,IOOptionBits options)
{
    char data[2];
    Xbox360Peripheral *owner = GetOwner(this);
    if (!owner) return kIOReturnUnsupported;
    report->readBytes(0, data, 2);
    switch(data[0]) {
        case 0x00:  // Set force feedback
            if((data[1]!=report->getLength()) || (data[1]!=0x04)) return kIOReturnUnsupported;
	    report->readBytes(2,data,2);
	    return owner->setRumble(data[0], data[1]);

        case 0x01:  // Set LEDs
            if((data[1]!=report->getLength())||(data[1]!=0x03)) return kIOReturnUnsupported;
	    report->readBytes(2,data,1);
	    return owner->setLeds(data[0]);

        default:
	    IOLog("Unknown escape %d\n", data[0]);
            return kIOReturnUnsupported;
    }
}

// Get report
IOReturn Xbox360ControllerClass::getReport(IOMemoryDescriptor *report,IOHIDReportType reportType,IOOptionBits options)
{
    // Doesn't do anything yet ;)
    //IOLog("%s\n", __FUNCTION__);
    return kIOReturnUnsupported;
}


OSString* Xbox360ControllerClass::newManufacturerString() const
{
    Xbox360Peripheral *owner = GetOwner(this);
    if (owner == NULL) return OSString::withCString("Unknown");
    return owner->getManufacturerString();
}

OSNumber* Xbox360ControllerClass::newPrimaryUsageNumber() const
{
    return OSNumber::withNumber(HID_360::ReportDescriptor[3], 8);
}

OSNumber* Xbox360ControllerClass::newPrimaryUsagePageNumber() const
{
    return OSNumber::withNumber(HID_360::ReportDescriptor[1], 8);
}

OSNumber* Xbox360ControllerClass::newProductIDNumber() const
{
    Xbox360Peripheral *owner = GetOwner(this);
    if (owner == NULL) return OSNumber::withNumber(-1,16);
    return owner->getProductIDNumber();
}

OSString* Xbox360ControllerClass::newProductString() const
{
    Xbox360Peripheral *owner = GetOwner(this);
    if (owner == NULL) return OSString::withCString("Unknown");
    return owner->getProductString();
}

OSString* Xbox360ControllerClass::newSerialNumberString() const
{
    Xbox360Peripheral *owner = GetOwner(this);
    if (owner == NULL) return OSString::withCString("Unknown");
    return owner->getSerialNumberString();
}

OSString* Xbox360ControllerClass::newTransportString() const
{
    return OSString::withCString("USB");
}

OSNumber* Xbox360ControllerClass::newVendorIDNumber() const
{
    Xbox360Peripheral *owner = GetOwner(this);
    if (owner == NULL) return OSNumber::withNumber(-1,16);
    return owner->getVendorIDNumber();
}

OSNumber* Xbox360ControllerClass::newLocationIDNumber() const
{
    Xbox360Peripheral *owner = GetOwner(this);
    if (owner == NULL) return OSNumber::withNumber(-1,16);
    return owner->getLocationIDNumber();
}
