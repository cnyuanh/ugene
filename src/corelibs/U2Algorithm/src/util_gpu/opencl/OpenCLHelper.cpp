#include "OpenCLHelper.h"

#include <U2Core/Log.h>


#include <stdio.h>

namespace U2 {

    const static char * clGetPlatformIDs_n ("clGetPlatformIDs");
    const static char * clGetPlatformInfo_n ("clGetPlatformInfo");
    const static char * clGetDeviceIDs_n ("clGetDeviceIDs");
    const static char * clGetDeviceInfo_n ("clGetDeviceInfo");

    const static char * clCreateContext_n ("clCreateContext");
    const static char * clCreateBuffer_n ("clCreateBuffer");
    const static char * clCreateProgramWithSource_n ("clCreateProgramWithSource");
    const static char * clGetProgramBuildInfo_n ("clGetProgramBuildInfo");
    const static char * clCreateKernel_n ("clCreateKernel");
    const static char * clSetKernelArg_n ("clSetKernelArg");
    const static char * clCreateCommandQueue_n ("clCreateCommandQueue");
    const static char * clEnqueueNDRangeKernel_n ("clEnqueueNDRangeKernel");
    const static char * clWaitForEvents_n ("clWaitForEvents");
    const static char * clEnqueueReadBuffer_n ("clEnqueueReadBuffer");
    const static char * clFinish_n ("clFinish");
    const static char * clBuildProgram_n ("clBuildProgram");
    const static char * clReleaseEvent_n ("clReleaseEvent");

    const static char * clReleaseKernel_n ("clReleaseKernel");
    const static char * clReleaseProgram_n ("clReleaseProgram");
    const static char * clReleaseCommandQueue_n ("clReleaseCommandQueue");
    const static char * clReleaseContext_n ("clReleaseContext");
    const static char * clReleaseMemObject_n ("clReleaseMemObject");

    OpenCLHelper::OpenCLHelper()  {

        coreLog.details( QObject::tr("Loading OPENCL driver library") );

        QLibrary openclLib( OPENCL_DRIVER_LIB );
        openclLib.load();
        if( !openclLib.isLoaded() ) {
            coreLog.details( QObject::tr("Cannot load OpenCL library. Error while loading %1").arg(openclLib.fileName()) );
            status = Error_NoDriverLib;
            return;
        }

        clGetPlatformIDs_p = clGetPlatformIDs_f( openclLib.resolve(clGetPlatformIDs_n));
        if( !clGetPlatformIDs_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clGetPlatformIDs_n) );
            status = Error_BadDriverLib;
            return;
        }

        clGetPlatformInfo_p = clGetPlatformInfo_f( openclLib.resolve(clGetPlatformInfo_n));
        if( !clGetPlatformInfo_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clGetPlatformInfo_n) );
            status = Error_BadDriverLib;
            return;
        }

        clGetDeviceIDs_p = clGetDeviceIDs_f( openclLib.resolve(clGetDeviceIDs_n));
        if( !clGetDeviceIDs_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clGetDeviceIDs_n) );
            status = Error_BadDriverLib;
            return;
        }

        clGetDeviceInfo_p = clGetDeviceInfo_f( openclLib.resolve(clGetDeviceInfo_n));
        if( !clGetDeviceInfo_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clGetDeviceInfo_n) );
            status = Error_BadDriverLib;
            return;
        }

        //****************************************

        clCreateContext_p = clCreateContext_f( openclLib.resolve(clCreateContext_n));
        if( !clCreateContext_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clCreateContext_n) );
            status = Error_BadDriverLib;
            return;
        }

        clCreateBuffer_p = clCreateBuffer_f( openclLib.resolve(clCreateBuffer_n));
        if( !clCreateBuffer_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clCreateBuffer_n) );
            status = Error_BadDriverLib;
            return;
        }

        clCreateProgramWithSource_p = clCreateProgramWithSource_f( openclLib.resolve(clCreateProgramWithSource_n));
        if( !clCreateProgramWithSource_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clCreateProgramWithSource_n) );
            status = Error_BadDriverLib;
            return;
        }

        clGetProgramBuildInfo_p = clGetProgramBuildInfo_f( openclLib.resolve(clGetProgramBuildInfo_n));
        if( !clGetProgramBuildInfo_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clGetProgramBuildInfo_n) );
            status = Error_BadDriverLib;
            return;
        }

        clCreateKernel_p = clCreateKernel_f( openclLib.resolve(clCreateKernel_n));
        if( !clCreateKernel_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clCreateKernel_n) );
            status = Error_BadDriverLib;
            return;
        }

        clSetKernelArg_p = clSetKernelArg_f( openclLib.resolve(clSetKernelArg_n));
        if( !clSetKernelArg_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clSetKernelArg_n) );
            status = Error_BadDriverLib;
            return;
        }

        clCreateCommandQueue_p = clCreateCommandQueue_f( openclLib.resolve(clCreateCommandQueue_n));
        if( !clCreateCommandQueue_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clCreateCommandQueue_n) );
            status = Error_BadDriverLib;
            return;
        }

        clEnqueueNDRangeKernel_p = clEnqueueNDRangeKernel_f( openclLib.resolve(clEnqueueNDRangeKernel_n));
        if( !clEnqueueNDRangeKernel_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clEnqueueNDRangeKernel_n) );
            status = Error_BadDriverLib;
            return;
        }

        clWaitForEvents_p = clWaitForEvents_f( openclLib.resolve(clWaitForEvents_n));
        if( !clWaitForEvents_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clWaitForEvents_n) );
            status = Error_BadDriverLib;
            return;
        }

        clEnqueueReadBuffer_p = clEnqueueReadBuffer_f( openclLib.resolve(clEnqueueReadBuffer_n));
        if( !clEnqueueReadBuffer_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clEnqueueReadBuffer_n) );
            status = Error_BadDriverLib;
            return;
        }

        clFinish_p = clFinish_f( openclLib.resolve(clFinish_n));
        if( !clFinish_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clFinish_n) );
            status = Error_BadDriverLib;
            return;
        }

        clBuildProgram_p = clBuildProgram_f( openclLib.resolve(clBuildProgram_n));
        if( !clBuildProgram_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clBuildProgram_n) );
            status = Error_BadDriverLib;
            return;
        }

        clReleaseEvent_p = clReleaseEvent_f( openclLib.resolve(clReleaseEvent_n));
        if( !clReleaseEvent_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clReleaseEvent_n) );
            status = Error_BadDriverLib;
            return;
        }

        clReleaseKernel_p = clReleaseKernel_f( openclLib.resolve(clReleaseKernel_n));
        if( !clReleaseKernel_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clReleaseKernel_n) );
            status = Error_BadDriverLib;
            return;
        }

        clReleaseProgram_p = clReleaseProgram_f( openclLib.resolve(clReleaseProgram_n));
        if( !clReleaseProgram_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clReleaseProgram_n) );
            status = Error_BadDriverLib;
            return;
        }

        clReleaseCommandQueue_p = clReleaseCommandQueue_f( openclLib.resolve(clReleaseCommandQueue_n));
        if( !clReleaseCommandQueue_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clReleaseCommandQueue_n) );
            status = Error_BadDriverLib;
            return;
        }

        clReleaseContext_p = clReleaseContext_f( openclLib.resolve(clReleaseContext_n));
        if( !clReleaseContext_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clReleaseContext_n) );
            status = Error_BadDriverLib;
            return;
        }

        clReleaseMemObject_p = clReleaseMemObject_f( openclLib.resolve(clReleaseMemObject_n));
        if( !clReleaseMemObject_p ) {
            coreLog.details( QObject::tr("Cannot resolve symbol %1").arg(clReleaseMemObject_n) );
            status = Error_BadDriverLib;
            return;
        }



        status = Error_NoError;
    }

    OpenCLHelper::~OpenCLHelper() {
    }

    QString OpenCLHelper::getErrorString() {
        switch (status) {
            case Error_NoDriverLib: {
                return QObject::tr("Cannot load library: %1").arg(OPENCL_DRIVER_LIB);
            }
            case Error_BadDriverLib: {
                return QObject::tr("Some errors occurs in library: %1").arg(OPENCL_DRIVER_LIB);
            }
            case Error_NoError: {
                return "";
            }
            default: {
                return "";
            }
        }
    }
} //namespace
