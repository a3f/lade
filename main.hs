module Main where

import Util.WinApi
import Data.Bits((.|.))
import Foreign.Ptr
import Data.Word
import Foreign.C.String

main :: IO ()
main = do
	process <- openProcess pROCESS_ALL_ACCESS False =<<
		getWindowThreadProcessId =<< findWindowX "TibiaClient" []
	let buffer = "D:\\_\\quake\\_.dll"
	putStr "Injecting into: "
	print process

	modHandle <- getModuleHandle $ Just "kernel32.dll"
	addr <- getProcAddress modHandle "LoadLibraryA"
	--addrKill <- getProcAddress modHandle "FreeLibraryA"
	arg <- virtualAllocEx process nullPtr ((fromIntegral $ length buffer) :: Word32)  (mEM_COMMIT .|. mEM_RESERVE) pAGE_READWRITE
	n <- writeProcessMemory process (arg) ((fromIntegral $ length buffer) :: Word32) =<< newCString buffer
	--m <- writeProcessMemory process (arg) ((fromIntegral $ length buffer) :: Word32) =<< newCString buffer

	-- HANDLE threadID = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, NULL, NULL);
	threadId <- createRemoteThread process addr arg
	--threadId <- createRemoteThread process addr arg

	print "Injection successful"
