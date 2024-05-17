/******************************************************************************
Module:  Handshake.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/
//Modified by a BSU fellow

#include "..\CmnHdr.h"     /* See Appendix A. */
#include <windowsx.h>
#include <tchar.h>
#include <process.h>       // For beginthreadex
#include <iostream>       // For beginthreadex
#include "Resource.h"
#include "..\MyDLL\MyDLL.h"


///////////////////////////////////////////////////////////////////////////////
HANDLE hThreadServer;

// This event is signaled when the client has a request for the server
HANDLE g_hevtRequestSubmitted;

// This event is signaled when the server has a result for the client
HANDLE g_hevtResultReturned;

// The buffer shared between the client and server threads
TCHAR  g_szSharedRequestAndResultBuffer[1024];

// The special value sent from the client that causes the 
// server thread to terminate cleanly.
TCHAR  g_szServerShutdown[] = TEXT("Server Shutdown");
TCHAR  addBuffer[] = TEXT("add");
TCHAR  multBuffer[] = TEXT("mult");


///////////////////////////////////////////////////////////////////////////////

BOOL fShutdown = FALSE;
// This is the code executed by the server thread
//Commonly used header is as follows:
//unsigned (__stdcall ServerThread(PVOID pvParam){...}
DWORD WINAPI ServerThread(PVOID pvParam) {

   // Assume that the server thread is to run forever
  fShutdown = FALSE;

   while (!fShutdown) {

      // Wait for the client to submit a request
      WaitForSingleObject(g_hevtRequestSubmitted, INFINITE);

      // Check to see if the client wants the server to terminate
      fShutdown = 
         (lstrcmpi(g_szSharedRequestAndResultBuffer, g_szServerShutdown) == 0);

      float result = 0;
      TCHAR input[100];
      std::sscanf(g_szSharedRequestAndResultBuffer, "%s", &input);
      float a = 0;
      std::sscanf(g_szSharedRequestAndResultBuffer, "%*s %f", &a);
      float b = 0;
      std::sscanf(g_szSharedRequestAndResultBuffer, "%*s %*f %f", &b);

      if (!fShutdown) {
          if (lstrcmpi(addBuffer, input) == 0) {
              result = add(a, b);
              std::snprintf(g_szSharedRequestAndResultBuffer, sizeof(g_szSharedRequestAndResultBuffer), "%f", result);
          }
          else if (lstrcmpi(multBuffer, input) == 0) {
              result = mult(a, b);
              std::snprintf(g_szSharedRequestAndResultBuffer, sizeof(g_szSharedRequestAndResultBuffer), "%f", result);
          }
          else {
              // Process the client's request (reverse the string)
              _tcsrev(g_szSharedRequestAndResultBuffer);//Look for it in tchar.h
          }
      }

      // Let the client process the request's result
      SetEvent(g_hevtResultReturned);
   }
	SetEvent(g_hevtResultReturned);
   // The client wants us to shutdown, exit
   return(0);
}


///////////////////////////////////////////////////////////////////////////////


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

   chSETDLGICONS(hwnd, IDI_HANDSHAKE);

   // Initialize the edit control with some test data request
   Edit_SetText(GetDlgItem(hwnd, IDC_REQUEST), TEXT("Some test data"));

   return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////
void Dlg_OnClose(HWND hwnd)
{
		 TerminateThread(hThreadServer,2);
	 // Wait for the server to process the request and give us the result
 //        WaitForSingleObject(hThreadServer, INFINITE);
 	EndDialog(hwnd, 0);
}

void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

   switch (id) {

      case IDCANCEL:
         EndDialog(hwnd, id);
         break;

      case IDC_SUBMIT:  // Submit a request to the server thread
		   if(fShutdown)break;
      
         // Copy the request string into the shared data buffer
         Edit_GetText(GetDlgItem(hwnd, IDC_REQUEST), //Look for it in Windowsx.h
            g_szSharedRequestAndResultBuffer, 
            chDIMOF(g_szSharedRequestAndResultBuffer));

         // Let the server thread know that a request is ready in the buffer
         SetEvent(g_hevtRequestSubmitted);

         // Wait for the server to process the request and give us the result
         WaitForSingleObject(g_hevtResultReturned, INFINITE);

         // Let the user know the result
         Edit_SetText(GetDlgItem(hwnd, IDC_RESULT), //Look for it in Windowsx.h
            g_szSharedRequestAndResultBuffer);
		 //

         break;
   }
}


///////////////////////////////////////////////////////////////////////////////


INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   
   switch (uMsg) {
      chHANDLE_DLGMSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
      chHANDLE_DLGMSG(hwnd, WM_COMMAND,    Dlg_OnCommand);
	  chHANDLE_DLGMSG(hwnd, WM_CLOSE,		Dlg_OnClose);
   }

   return(FALSE);
}


///////////////////////////////////////////////////////////////////////////////


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) {

   // Create & initialize the 2 nonsignaled, auto-reset events

   g_hevtRequestSubmitted = CreateEvent(
	   NULL,//If lpEventAttributes is NULL, the handle cannot be inherited. 
	   FALSE,
//If TRUE, then you must use the ResetEvent function to manually reset the state to nonsignaled.
// If FALSE, the system automatically resets the state to nonsignaled 
//after a single waiting thread has been released. 
	   FALSE,//If TRUE, the initial state is signaled; otherwise, it is nonsignaled. 
	   NULL);//If lpName is NULL, the event object is created without a name. 


   g_hevtResultReturned   = CreateEvent(NULL, FALSE, FALSE, NULL);

   // Spawn the server thread


   typedef unsigned (__stdcall *PTHREAD_START) (void *);
   DWORD dwThreadID;
//////////////////////////
/////fShutdown = FALSE;   /////
//////////////////////////

 hThreadServer = (HANDLE)_beginthreadex(NULL, 0,(PTHREAD_START) ServerThread, NULL, 
      0, (unsigned int *)&dwThreadID);//

//   HANDLE hThreadServer = chBEGINTHREADEX(NULL, 0, ServerThread, NULL, 
//      0, &dwThreadID);

   // Execute the client thread's user-interface
   DialogBox(hinstExe, MAKEINTRESOURCE(IDD_HANDSHAKE), NULL, (DLGPROC)Dlg_Proc);

   // The client's UI is closing, have the server thread shutdown
//   lstrcpy(g_szSharedRequestAndResultBuffer, g_szServerShutdown);
//   SetEvent(g_hevtRequestSubmitted);

   // Wait for the server thread to acknowledge the shutdown AND
   // wait for the server thread to fully terminate

   HANDLE h[2];
   h[0] = g_hevtResultReturned;
   h[1] = hThreadServer;
   WaitForMultipleObjects(2, h, FALSE, INFINITE);//TRUE is wrong!

   // Properly clean up everything
   CloseHandle(hThreadServer);      
   CloseHandle(g_hevtRequestSubmitted);      
   CloseHandle(g_hevtResultReturned);      

   // The client thread terminates with the whole process
   return(0);
}


//////////////////////////////// End of File //////////////////////////////////
