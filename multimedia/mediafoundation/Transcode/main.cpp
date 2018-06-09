//////////////////////////////////////////////////////////////////////////
//
// main.cpp - Defines the entry point for the console application.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// This sample demonstrates how to perform simple transcoding
// to WMA or WMV.
//
////////////////////////////////////////////////////////////////////////// 

#include "Transcode.h"
#include <Shlwapi.h>
#include <tchar.h>
#include <string>
#include <sstream>

#pragma comment(lib, "Shlwapi.lib")

struct FileExtension {
	LPCTSTR description;
	LPCTSTR extension;
	LPCGUID audioSubType;
	LPCGUID videoSubType;
	LPCGUID containerType;
};

// String with 2 terminating null characters.
// Used to concatenate multiple strings into a string.
#define EXT(x) _T(#x) _T("\0")

static const FileExtension fileExtensions[] = {
	{
		_T("MPEG4"), EXT(.mp4) EXT(.m4v),
		&MFAudioFormat_AAC, &MFVideoFormat_H264, &MFTranscodeContainerType_MPEG4
	},
	{
		_T("MPEG Audio"), EXT(.m4a),
		&MFAudioFormat_AAC, nullptr, &MFTranscodeContainerType_MPEG4
	},
	{
		_T("Windows Media Video"), EXT(.wmv) EXT(.asf),
		&MFAudioFormat_WMAudioV9, &MFVideoFormat_WMV3, &MFTranscodeContainerType_ASF
	},
	{
		_T("Windows Media Audio"), EXT(.wma),
		&MFAudioFormat_WMAudioV9, nullptr, &MFTranscodeContainerType_ASF
	},
};

static std::wstring availableExtensions();
static LPCTSTR nextString(LPCTSTR str);

int wmain(int argc, wchar_t* argv[])
{
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (argc != 3)
    {
        wprintf_s(
			L"Usage: %s input_file output_file\n"
			L"Following extensions are available for output_file\n"
			L"%s",
			argv[0], availableExtensions().c_str());
        return 0;
    }

    const WCHAR* sInputFile = argv[1];  // Audio source file name
    const WCHAR* sOutputFile = argv[2];  // Output file name
    
	// Determine output sub type and container type from extension of output file.
	TCHAR fileName[MAX_PATH];
	_tcscpy_s(fileName, sOutputFile);
	auto ext = PathFindExtension(fileName);
	const FileExtension* fileExtention = nullptr;
	if(ext && *ext) {
		for(auto& fe : fileExtensions) {
			auto extension = fe.extension;
			while(extension) {
				if(0 == _tcsicmp(ext, extension)) {
					fileExtention = &fe;
					break;
				}
				extension = nextString(extension);
			}
			if(fileExtention) break;
		}
	}
	if(!fileExtention) {
		wprintf_s(L"Unknown extension of output file name.\n");
		return 1;
	}

    HRESULT hr = S_OK;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        hr = MFStartup(MF_VERSION);
    }

    if (SUCCEEDED(hr))
    {
        CTranscoder transcoder;

        // Create a media source for the input file.
        hr = transcoder.OpenFile(sInputFile);

        if (SUCCEEDED(hr))
        {
            wprintf_s(L"Opened file: %s.\n", sInputFile);

            //Configure the profile and build a topology.
            hr = transcoder.ConfigureAudioOutput(fileExtention->audioSubType);
        }

        if (SUCCEEDED(hr))
        {
            hr = transcoder.ConfigureVideoOutput(fileExtention->videoSubType);
        }
    
        if (SUCCEEDED(hr))
        {
            hr = transcoder.ConfigureContainer(fileExtention->containerType);
        }

        //Transcode and generate the output file.

        if (SUCCEEDED(hr))
        {
            hr = transcoder.EncodeToFile(sOutputFile);
        }
    
        if (SUCCEEDED(hr))
        {
            wprintf_s(L"Output file created: %s\n", sOutputFile);
        }
    }

    MFShutdown();
    CoUninitialize();

    if (FAILED(hr))
    {
        wprintf_s(L"Could not create the output file (0x%X).\n", hr);
    }

    return 0;
}

// Returns string contains description and extensions.
/*static*/ std::wstring availableExtensions()
{
	std::wstringstream stream;
	for(auto fe : fileExtensions) {
		auto extension = fe.extension;
		while(extension) {
			stream << L" " << extension;
			extension = nextString(extension);
		}
		stream << L": " << fe.description << std::endl;
	}
	return stream.str();
}

/*static*/ LPCTSTR nextString(LPCTSTR str)
{
	return *str ? (str + _tcslen(str) + 1) : nullptr;
}
