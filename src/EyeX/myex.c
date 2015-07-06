/*
* myex v0.0.1 [13/01/2015]
* A simple proto-toolbox to act as a middleman between Matlab and the EyeX Engine. Adapted from MinimalGazeDataStream.c.
*
* Incoming data is stored asynchronously (via callbacks) in a stack (implemented as a linked list).
* The buffer can be manually polled by Matlab, at which point the buffer is cleared.
*
* Data is returned in a matrix. Each row contains TX_GAZEPOINTDATAEVENTPARAMS and TX_EYEPOSITIONDATAEVENTPARAMS data, thus:
*        X (px)
*        Y (px)
*        EyeGazeTimestamp (microseconds)
*        HasLeftEyePosition (0 or 1)
*        HasRightEyePosition (0 or 1)
*        LeftEyeX (mm)
*        LeftEyeY (mm)
*        LeftEyeZ (mm)
*        RightEyeX (mm)
*        RightEyeY (mm)
*        RightEyeZ (mm)
*        EyePosTimestamp (microseconds)
*
* This code works, but has not been extensively debugged or tested, and could no doubt be improved. It is intended
* as a proof-of-principle only.
*
*     Commands are:
*        eyex('connect')
*        eyex('get')
*        eyex('disconnect')
*
* Copyright 2015 Pete Jones <petejonze@gmail.com>
*/
/* mod. by Eiichiro Momma 2015/07/06 */
 
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include "eyex\EyeX.h"
#include "mex.h"
 
#pragma comment (lib, "Tobii.EyeX.Client.lib")
 
// ID of the global interactor that provides our data stream; must be unique within the application.
static const TX_STRING InteractorId = "Twilight Sparkle";
 
// global variables
static TX_HANDLE g_hGlobalInteractorSnapshot = TX_EMPTY_HANDLE;
static TX_CONTEXTHANDLE hContext = TX_EMPTY_HANDLE;
 
/*
* INTERNAL DATA STORAGE -------------------------------------------------------------------------------
*/
// temporarily store last known eye position
TX_EYEPOSITIONDATAEVENTPARAMS lastKnownEyePositionData;
 
// stack data type
struct node
{
    TX_REAL X_px;
    TX_REAL Y_px;
    TX_REAL Timestamp; // For TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED this is the point in time when the filter was applied. For TX_GAZEPOINTDATAMODE_UNFILTERED this is the point in time time when gazepoint was captured.
    TX_BOOL HasLeftEyePosition;
    TX_BOOL HasRightEyePosition;
    TX_REAL LeftEyeX_mm;
    TX_REAL LeftEyeY_mm;
    TX_REAL LeftEyeZ_mm;
    TX_REAL RightEyeX_mm;
    TX_REAL RightEyeY_mm;
    TX_REAL RightEyeZ_mm;
    TX_REAL EyePosTimestamp; //The point in time when the eye position was captured.
    struct node *ptr;
}*top,*top1,*temp;
int count = 0;
 
/* Create empty stack */
void q_create()
{
    top = NULL;
}
/* Push data into stack */
void q_push(TX_GAZEPOINTDATAEVENTPARAMS GazeData, TX_EYEPOSITIONDATAEVENTPARAMS lastKnownEyePositionData)
{
    if (top == NULL)
    {
        top =(struct node *)malloc(1*sizeof(struct node));
        top->ptr = NULL;
        top->X_px = GazeData.X;
        top->Y_px = GazeData.Y;
        top->Timestamp = GazeData.Timestamp;
        top->HasLeftEyePosition = lastKnownEyePositionData.HasLeftEyePosition;
        top->HasRightEyePosition = lastKnownEyePositionData.HasRightEyePosition;
        top->LeftEyeX_mm = lastKnownEyePositionData.LeftEyeX;
        top->LeftEyeY_mm = lastKnownEyePositionData.LeftEyeY;
        top->LeftEyeZ_mm = lastKnownEyePositionData.LeftEyeZ;
        top->RightEyeX_mm = lastKnownEyePositionData.RightEyeX;
        top->RightEyeY_mm = lastKnownEyePositionData.RightEyeY;
        top->RightEyeZ_mm = lastKnownEyePositionData.RightEyeZ;
        top->EyePosTimestamp = lastKnownEyePositionData.Timestamp;
    }
    else
    {
        temp =(struct node *)malloc(1*sizeof(struct node));
        temp->ptr = top;
        temp->X_px = GazeData.X;
        temp->Y_px = GazeData.Y;
        temp->Timestamp = GazeData.Timestamp;
        temp->HasLeftEyePosition = lastKnownEyePositionData.HasLeftEyePosition;
        temp->HasRightEyePosition = lastKnownEyePositionData.HasRightEyePosition;
        temp->LeftEyeX_mm = lastKnownEyePositionData.LeftEyeX;
        temp->LeftEyeY_mm = lastKnownEyePositionData.LeftEyeY;
        temp->LeftEyeZ_mm = lastKnownEyePositionData.LeftEyeZ;
        temp->RightEyeX_mm = lastKnownEyePositionData.RightEyeX;
        temp->RightEyeY_mm = lastKnownEyePositionData.RightEyeY;
        temp->RightEyeZ_mm = lastKnownEyePositionData.RightEyeZ;
        temp->EyePosTimestamp = lastKnownEyePositionData.Timestamp;        
        top = temp;
    }
    count++;
}
/* Pop Operation on stack */
void q_pop()
{
    top1 = top;
    if (top1 == NULL)
    {
        mexErrMsgIdAndTxt("eyex:buffer:indexError", "Error : Trying to pop from empty stack.");
        return;
    }
    else
    {
        top1 = top1->ptr;
        free(top);
        top = top1;
        count--;
    }
}
/* Return top element */
struct node *q_topelement()
{
    return(top);
}
/* Check if stack is empty or not */
int q_isempty()
{
    return(top == NULL);
}
/* Destroy entire stack */
void q_destroy()
{
    top1 = top;
    while (top1 != NULL)
    {
        top1 = top->ptr;
        free(top);
        top = top1;
        top1 = top1->ptr;
    }
    free(top1);
    top = NULL;
    count = 0;
    //printf("\n All stack elements destroyed\n");
}
 
/* Create empty stack */
int q_nelements()
{
    return(count);
}
 
/*
* EYEX ENGINE INTERFACE -------------------------------------------------------------------------------
*/
 
/*
* Initializes g_hGlobalInteractorSnapshot with an interactor that has the Gaze Point behavior.
*/
BOOL InitializeGlobalInteractorSnapshot(TX_CONTEXTHANDLE hContext)
{
    TX_HANDLE hInteractor = TX_EMPTY_HANDLE;
    TX_HANDLE hBehavior   = TX_EMPTY_HANDLE;
    TX_HANDLE hBehaviorWithoutParameters = TX_EMPTY_HANDLE;
    TX_GAZEPOINTDATAPARAMS params = { TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED };
    BOOL success;
 
    success = txCreateGlobalInteractorSnapshot(
        hContext,
        InteractorId,
        &g_hGlobalInteractorSnapshot,
        &hInteractor) == TX_RESULT_OK;
    success &= txCreateInteractorBehavior(hInteractor, &hBehavior, TX_BEHAVIORTYPE_GAZEPOINTDATA) == TX_RESULT_OK;
    success &= txSetGazePointDataBehaviorParams(hBehavior, &params) == TX_RESULT_OK;
 
    // add a second behavior to the same interactor: eye position data.
    // this one is a bit different because it doesn't take any parameters.
    // therefore we use the generic txCreateInteractorBehavior function (and remember to release the handle!)
    success &= txCreateInteractorBehavior(hInteractor, &hBehaviorWithoutParameters, TX_BEHAVIORTYPE_EYEPOSITIONDATA) == TX_RESULT_OK;
    
    // release the handles
    txReleaseObject(&hBehavior);
    txReleaseObject(&hBehaviorWithoutParameters);
    txReleaseObject(&hInteractor);
 
    return success;
}
 
/*
* Callback function invoked when a snapshot has been committed.
*/
void TX_CALLCONVENTION OnSnapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param)
{
    // check the result code using an assertion.
    // this will catch validation errors and runtime errors in debug builds. in release builds it won't do anything.
 
    TX_RESULT result = TX_RESULT_UNKNOWN;
    txGetAsyncDataResultCode(hAsyncData, &result);
    assert(result == TX_RESULT_OK || result == TX_RESULT_CANCELLED);
}
 
/*
* Callback function invoked when the status of the connection to the EyeX Engine has changed.
*/
void TX_CALLCONVENTION OnEngineConnectionStateChanged(TX_CONNECTIONSTATE connectionState, TX_USERPARAM userParam)
{
    switch (connectionState) {
    case TX_CONNECTIONSTATE_CONNECTED: {
            BOOL success;
            mexPrintf("The connection state is now CONNECTED (We are connected to the EyeX Engine)\n");
            // commit the snapshot with the global interactor as soon as the connection to the engine is established.
            // (it cannot be done earlier because committing means "send to the engine".)
            success = txCommitSnapshotAsync(g_hGlobalInteractorSnapshot, OnSnapshotCommitted, NULL) == TX_RESULT_OK;
            if (!success) {
                mexPrintf("Failed to initialize the data stream.\n");
            }
            else
            {
                mexPrintf("Waiting for gaze data to start streaming...\n");
            }
        }
        break;
 
    case TX_CONNECTIONSTATE_DISCONNECTED:
        mexPrintf("The connection state is now DISCONNECTED (We are disconnected from the EyeX Engine)\n");
        break;
 
    case TX_CONNECTIONSTATE_TRYINGTOCONNECT:
        mexPrintf("The connection state is now TRYINGTOCONNECT (We are trying to connect to the EyeX Engine)\n");
        break;
 
    case TX_CONNECTIONSTATE_SERVERVERSIONTOOLOW:
        mexPrintf("The connection state is now SERVER_VERSION_TOO_LOW: this application requires a more recent version of the EyeX Engine to run.\n");
        break;
 
    case TX_CONNECTIONSTATE_SERVERVERSIONTOOHIGH:
        mexPrintf("The connection state is now SERVER_VERSION_TOO_HIGH: this application requires an older version of the EyeX Engine to run.\n");
        break;
    }
}
 
/*
* Handles an event from the Gaze Point data stream.
*/
void OnGazeDataEvent(TX_HANDLE hGazeDataBehavior)
{
    TX_GAZEPOINTDATAEVENTPARAMS eventParams;
    if (txGetGazePointDataEventParams(hGazeDataBehavior, &eventParams) == TX_RESULT_OK) {
        //mexPrintf("Pushing in Gaze Data: (%.1f, %.1f) Timestamp %.0f ms\n", eventParams.X, eventParams.Y, eventParams.Timestamp);
        q_push (eventParams, lastKnownEyePositionData);    
    } else {
        mexPrintf("Failed to interpret gaze data event packet.\n");
    }
}
 
/*
* Handles an event from the EyePosition data stream.
*/
void OnEyepositionDataEvent(TX_HANDLE hGazeDataBehavior)
{
    TX_EYEPOSITIONDATAEVENTPARAMS eventParams;
    if (txGetEyePositionDataEventParams(hGazeDataBehavior, &eventParams) == TX_RESULT_OK) {
        //printf("Has Left Eye Position: %d\n", eventParams.HasLeftEyePosition);
        lastKnownEyePositionData = eventParams;
    } else {
        printf("Failed to interpret gaze data event packet.\n");
        printf ("Error code: %d.\n", txGetEyePositionDataEventParams(hGazeDataBehavior, &eventParams));
    }
}
 
/*
* Callback function invoked when an event has been received from the EyeX Engine.
*/
void TX_CALLCONVENTION HandleEvent(TX_CONSTHANDLE hAsyncData, TX_USERPARAM userParam)
{
    TX_HANDLE hEvent = TX_EMPTY_HANDLE;
    TX_HANDLE hBehavior = TX_EMPTY_HANDLE;
 
    txGetAsyncDataContent(hAsyncData, &hEvent);
 
    // NOTE. Uncomment the following line of code to view the event object. The same function can be used with any interaction object.
    OutputDebugStringA(txDebugObject(hEvent));
 
    if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_GAZEPOINTDATA) == TX_RESULT_OK) {
        OnGazeDataEvent(hBehavior);
        txReleaseObject(&hBehavior);
    } else if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_EYEPOSITIONDATA) == TX_RESULT_OK) {
        OnEyepositionDataEvent(hBehavior);
        txReleaseObject(&hBehavior);
    }
 
    // NOTE since this is a very simple application with a single interactor and a single data stream,
    // our event handling code can be very simple too. A more complex application would typically have to
    // check for multiple behaviors and route events based on interactor IDs.
 
    txReleaseObject(&hEvent);
}
 
/*
* MATLAB ENGINE INTERFACE -------------------------------------------------------------------------------
*/
 
void Connect(void)
{
 
    // initialize variables
    TX_TICKET hConnectionStateChangedTicket = TX_INVALID_TICKET;
    TX_TICKET hEventHandlerTicket = TX_INVALID_TICKET;
    BOOL success;
 
    // create data stack
    q_create();
 
    // initialize and enable the context that is our link to the EyeX Engine.
    success = txInitializeEyeX(TX_EYEXCOMPONENTOVERRIDEFLAG_NONE, NULL, NULL, NULL, NULL) == TX_RESULT_OK;
    success &= txCreateContext(&hContext, TX_FALSE) == TX_RESULT_OK;
    success &= InitializeGlobalInteractorSnapshot(hContext);
    success &= txRegisterConnectionStateChangedHandler(hContext, &hConnectionStateChangedTicket, OnEngineConnectionStateChanged, NULL) == TX_RESULT_OK;
    success &= txRegisterEventHandler(hContext, &hEventHandlerTicket, HandleEvent, NULL) == TX_RESULT_OK;
    success &= txEnableConnection(hContext) == TX_RESULT_OK;
    
    // short pause to try and ensure that the callbacks are fired and the printf messages sent to Matlab before this function terminates
    Sleep(100);
}
 
void Disconnect(void)
{
    // disable and delete the context.
    txDisableConnection(hContext);
    txReleaseObject(&g_hGlobalInteractorSnapshot);
    txShutdownContext(hContext, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE);
    txReleaseContext(&hContext);
    
    q_destroy();
}
 
/*
* Application entry point.
*/
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    char *string_command;
    double *outArray;
    mwSize M; //mwSize is a platform independent alternative to int
    mwSize i;
    
    if(nrhs!=1) {
        mexErrMsgIdAndTxt("myex:nrhs:invalidN", "Invalid input: One command string required.\n\nValid commands are:\n   myex('connect')\n   myex('getdata')\n   myex('disconnect')");
    }
 
    string_command = mxArrayToString(prhs[0]);
    //mexPrintf(">> %s\n", string_command);
    switch (string_command[0]) {
    case 'c':
        Connect();
        break;
    case 'g':
        // retrieve (any) data from the internal buffer - return as plhs[0]
        M = q_nelements();
        i = M; // start at the end of the output array, because we're actually using a FILO data structure for the internal buffer (stack)
        plhs[0] = mxCreateDoubleMatrix(q_nelements(),12,mxREAL);
        outArray = mxGetPr(plhs[0]);
        // iterate through internal buffer
        while (!q_isempty())
        {
            // get next item from the internal buffer
            struct node *topelement = q_topelement();
            i--;
            // add gaze data to output row
            outArray[i] = topelement->X_px;
            outArray[i+M] = topelement->Y_px;  // N.B. C indexing is like the tranpose of MATLAB variable: http://uk.mathworks.com/matlabcentral/newsreader/view_thread/249954
            outArray[i+2*M] = topelement->Timestamp;
            // add eye position data to output row
            outArray[i+3*M] = topelement->HasLeftEyePosition;
            outArray[i+4*M] = topelement->HasRightEyePosition;
            outArray[i+5*M] = topelement->LeftEyeX_mm;
            outArray[i+6*M] = topelement->LeftEyeY_mm;
            outArray[i+7*M] = topelement->LeftEyeZ_mm;
            outArray[i+8*M] = topelement->RightEyeX_mm;
            outArray[i+9*M] = topelement->RightEyeY_mm;
            outArray[i+10*M] = topelement->RightEyeZ_mm;
            outArray[i+11*M] = topelement->EyePosTimestamp;
            // remove this item from the internal buffer
            q_pop();
        }
        break;
    case 'd':
        Disconnect();        
        break;        
    default:
        mexErrMsgIdAndTxt("myex:nrhs:unrecognised", "Invalid input: Unrecognised command.\n\nValid commands are:\n   myex('connect')\n   myex('getdata')\n   myex('disconnect')");
        break;
    }
 
    mxFree(string_command);    
    return;
}