module Util.WinApi where
import Graphics.Win32.GDI.Types(HWND)
import System.Win32.Types (withTString, ptrToMaybe, LPCTSTR, DWORD)
import Foreign.C.String
import Control.Monad (liftM)
import Foreign
import System.Win32.Types
import Foreign.C (CInt, CUInt)
--import Foreign.Ptr

findWindowW :: String -> String -> IO (Maybe HWND)
findWindowW cname wname =
	withTString cname $ \ c_cname ->
	withTString wname $ \ c_wname ->
	liftM ptrToMaybe $ c_FindWindowW (if cname == "" then nullPtr else c_cname) 
									 (if wname == "" then nullPtr else c_wname)
foreign import stdcall unsafe "windows.h FindWindowW"
	c_FindWindowW :: LPCTSTR -> LPCTSTR -> IO HWND

findWindowX :: String -> String -> IO HWND
findWindowX cname wname =
	withTString cname $ \ c_cname ->
	withTString wname $ \ c_wname ->
	c_FindWindowW (if cname == "" then nullPtr else c_cname) 
  				  (if wname == "" then nullPtr else c_wname)

getWindowThreadProcessId :: HWND -> IO ProcessId
getWindowThreadProcessId hwnd =
	alloca $ \pid -> do 
		c_getWindowThreadProcessId hwnd pid
		(return.fromIntegral =<< peek (pid :: Ptr CUInt))
-- failWhenNULL !!!!
foreign import stdcall unsafe "windows.h GetWindowThreadProcessId"
	c_getWindowThreadProcessId :: HWND -> Ptr CUInt -> IO Int

foreign import stdcall unsafe "windows.h OpenProcess"
    c_OpenProcess :: ProcessAccessRights -> BOOL -> ProcessId -> IO ProcessHandle


openProcess :: ProcessAccessRights -> BOOL -> ProcessId -> IO ProcessHandle
openProcess r inh i = failIfNull "OpenProcess" $ c_OpenProcess r inh i

type ProcessId = DWORD
type ProcessHandle = HANDLE
type ProcessAccessRights = DWORD
pROCESS_ALL_ACCESS             :: ProcessAccessRights
pROCESS_ALL_ACCESS             =  2035711

getProcAddress :: HMODULE -> String -> IO Addr
getProcAddress hmod procname =
  withCAString procname $ \ c_procname ->
  failIfNull "GetProcAddress" $ c_GetProcAddress hmod c_procname
foreign import stdcall unsafe "windows.h GetProcAddress"
  c_GetProcAddress :: HMODULE -> LPCSTR -> IO Addr

getModuleHandle :: Maybe String -> IO HMODULE
getModuleHandle mb_name =
  maybeWith withTString mb_name $ \ c_name ->
  failIfNull "GetModuleHandle" $ c_GetModuleHandle c_name
foreign import stdcall unsafe "windows.h GetModuleHandleW"
  c_GetModuleHandle :: LPCTSTR -> IO HMODULE

loadLibrary :: String -> IO HINSTANCE
loadLibrary name =
  withTString name $ \ c_name ->
  failIfNull "LoadLibrary" $ c_LoadLibrary c_name
foreign import stdcall unsafe "windows.h LoadLibraryW"
  c_LoadLibrary :: LPCTSTR -> IO HINSTANCE

type VirtualAllocFlags = DWORD

mEM_COMMIT    :: VirtualAllocFlags
mEM_COMMIT    =  4096
mEM_RESERVE   :: VirtualAllocFlags
mEM_RESERVE   =  8192

{- ToIntegral!!!!!!!!!! -}
virtualAllocEx :: ProcessHandle -> Addr -> DWORD -> VirtualAllocFlags -> ProtectFlags -> IO Addr
virtualAllocEx hnd addt size ty flags =
  failIfNull "VirtualAllocEx" $ c_VirtualAllocEx hnd addt size ty flags
foreign import stdcall unsafe "windows.h VirtualAllocEx"
  c_VirtualAllocEx :: ProcessHandle -> Addr -> DWORD -> DWORD -> DWORD -> IO Addr

type ProtectFlags = DWORD

pAGE_EXECUTE_READWRITE  :: ProtectFlags
pAGE_EXECUTE_READWRITE  =  64


writeProcessMemory :: ProcessHandle -> Ptr () -> DWORD -> Ptr a -> IO ()
writeProcessMemory proc addr size buf =
    failIf_ not "writeProcessMemory: WriteProcessMemory" $
        c_WriteProcessMemory proc addr (castPtr buf) (fromIntegral size) nullPtr

foreign import stdcall "windows.h WriteProcessMemory" c_WriteProcessMemory ::
    ProcessHandle -> Ptr () -> Ptr Word8 -> DWORD -> Ptr DWORD -> IO BOOL

-- HANDLE threadID = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, NULL, NULL);

createRemoteThread :: ProcessHandle -> Addr -> Addr -> IO ProcessHandle
createRemoteThread proc addr arg =
    failIfNull "createRemoteThread: CreateRemoteThread" $
        c_CreateRemoteThread proc nullPtr 0 addr arg 0 nullPtr
--	threadId = createRemoteThread process nullPtr 0 addr arg 0 nullPtr
foreign import stdcall unsafe "windows.h CreateRemoteThread" 
	c_CreateRemoteThread :: ProcessHandle -> Ptr () -> DWORD -> Addr -> Addr -> DWORD -> Ptr () -> IO ProcessHandle
