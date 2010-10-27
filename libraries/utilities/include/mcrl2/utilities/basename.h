// Author(s): Frank Stappers 
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/utilities/basename.h
/// \brief Base class for tools that use a data rewriter.

#ifndef MCRL2_BASENAME_H
#define MCRL2_BASENAME_H


  #include <stdio.h>
  #include <string>
  #include <iostream>

  #ifdef _WIN32
    #include <windows.h>
  #endif //_WIN32

  #ifdef __APPLE__                                                                                                             
    #include <Carbon/Carbon.h>                                                                                                 
  #endif           
  
  namespace mcrl2 {
  
  namespace utilities {

  class basename 
  {
  public:
 
    /// \brief Returns the basename of a tool.
    /// \return A string 
    std::string get_executable_basename()
    {
      std::string path;
      #ifdef __linux
        path = "";
        pid_t pid = getpid();
        char buf[10];
        sprintf(buf,"%d",pid);
        std::string _link = "/proc/";
        _link.append( buf );
        _link.append( "/exe");
        char proc[512];
        int ch = readlink(_link.c_str(),proc,512);
        if (ch != -1) {
          proc[ch] = 0;
          path = proc;
          std::string::size_type t = path.find_last_of("/");
          path = path.substr(0,t);
        }
      #endif // __linux
    
      #ifdef __APPLE__
        /* path = "./";
        ProcessSerialNumber PSN;
        ProcessInfoRec pinfo;
        FSSpec pspec;
        FSRef fsr;
        OSStatus err;
    
        / * set up process serial number * /
        PSN.highLongOfPSN = 0;
        PSN.lowLongOfPSN = kCurrentProcess;
    
        / * set up info block * /
        pinfo.processInfoLength = sizeof(pinfo);
        pinfo.processName = NULL;
        pinfo.processAppSpec = &pspec;
    
        / * grab the vrefnum and directory * /
        err = GetProcessInformation(&PSN, &pinfo);
        if (! err ) {
          char c_path[2048];
          FSSpec fss2;
          int tocopy;
          err = FSMakeFSSpec(pspec.vRefNum, pspec.parID, 0, &fss2);
          if ( ! err ) 
          {
            err = FSpMakeFSRef(&fss2, &fsr);
            if ( ! err ) 
            {
              char c_path[2049];
              err = (OSErr)FSRefMakePath(&fsr, (UInt8*)c_path, 2048);
              if (! err ) 
              {
                path = c_path;
              }
            }
          }
        }
        std::string::size_type t = path.size();
        for( int i = 0; i < 3; ++i)
        {
          t = path.find_last_of("/", t-1);
        }
        path = path.substr(0,t); */

        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFURLRef resourcesURL = CFBundleCopyBundleURL(mainBundle);
        CFStringRef str = CFURLCopyFileSystemPath( resourcesURL, kCFURLPOSIXPathStyle );
        CFRelease(resourcesURL);
        char cpath[PATH_MAX];
  
        CFStringGetCString( str, cpath, FILENAME_MAX, kCFStringEncodingASCII );
        CFRelease(str);
        path = cpath;

      #endif //__APPLE__
    
      #ifdef _WIN32
        char buffer[MAX_PATH];//always use MAX_PATH for filepaths
        GetModuleFileName(NULL,buffer,sizeof(buffer));
        path = buffer;
        std::string::size_type t = path.find_last_of("\\");
        path = path.substr(0,t);
      #endif // _WIN32
    
      return path;
    };
  
  
    #ifdef _WIN32
  
    /// \brief Returns the install path of the toolset for WIN32
    /// \return A string 
    std::string get_win32_install_path()
    {	  
      std::string install_path;
      HKEY hKey = 0;
      char buf[255] = {0};
      DWORD dwBufSize = sizeof(buf);
      DWORD dwType;
   
      // TODO:
  	  // 
  	  // Following line has to become VENDOR and INSTALL_REGISTRY_KEY
      //   if( RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\"+VENDOR+"\\"+ INSTALL_REGISTRY_KEY),&hKey) == ERROR_SUCCESS)
  	  // Instead of: 
      if( RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\TUe\\mCRL2"),&hKey) == ERROR_SUCCESS)
      {
        dwType = REG_SZ;
        if( RegQueryValueEx(hKey,"",0, &dwType, (BYTE*)buf, &dwBufSize) == ERROR_SUCCESS)
        {
  							install_path = buf;
        }
  	    std::cerr << "Cannot find default value for key: HKEY_LOCAL_MACHINE\\SOFTWARE\\TUe\\mCRL2 \n" ;
  	  }
      else 
      { 
    	  std::cerr << "Cannot find key for HKEY_LOCAL_MACHINE\\SOFTWARE\\TUe\\mCRL2 \n" ;
      }
      return install_path; 
    };   
    #endif //_WIN32
  
  
    /// \brief Returns the basename of the toolset if installed, otherwise returns basename of executable
    /// \return A string 
    std::string get_toolset_basename()
    {
        std::string path = get_executable_basename();
        std::string::size_type t = path.size();

#ifdef _WIN32
	t = path.find_last_of("\\bin", t-4);
#else
	t = path.find_last_of("/bin", t-4);
#endif	
  
        if(  t + 4 == path.size() ){
          path = path.substr(0,t);
        }
        return path;
    };	  
  };

  } 
}
  
#endif //MCRL2_BASENAME_H
