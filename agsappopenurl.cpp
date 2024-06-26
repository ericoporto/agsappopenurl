/*
 * MIT License
 *
 * Copyright (c) 2024 Érico Porto
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define MAX_URL_SIZE 2048

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// DllMain - standard Windows DLL entry point.
// The AGS editor will cause this to get called when the editor first
// starts up, and when it shuts down at the end.
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  return TRUE;
}
#endif // _WIN32

#define THIS_IS_THE_PLUGIN
#include "agsappopenurl.h"
#include <algorithm>
#include <string>
#include "SDL.h"

// ***** DESIGN TIME CALLS *******
IAGSEditor *editor;
const char *ourScriptHeader =
  "#define AGS_APPOPENURL_VERSION 1\r\n"
  "enum AgsUrlProtocol {\r\n"
  " eAUrlProto_https = 0,\r\n"
  " eAUrlProto_http\r\n"
  "};\r\n"
  "import bool AppOpenURL(AgsUrlProtocol protocol, const string url);\r\n";

int AGS_EditorStartup(IAGSEditor *lpEditor)
{
  if (lpEditor->version < 1)
    return -1;

  editor = lpEditor;
  editor->RegisterScriptHeader(ourScriptHeader);
  return 0; // Return 0 to indicate success
}

void AGS_EditorShutdown()
{
  editor->UnregisterScriptHeader(ourScriptHeader);
}

void AGS_EditorProperties(HWND parent) {}
int AGS_EditorSaveGame(char *buffer, int bufsize) { return 0; }
void AGS_EditorLoadGame(char *buffer, int bufsize) {}

// ******* END DESIGN TIME  *******

// ****** RUN TIME ********
enum AgsUrlProtocol {
    eAgsUrlProt_https = 0,
    eAgsUrlProt_http
};
IAGSEngine *engine;

#define FAIL_LOG_AND_EXIT(X) \
do {                    \
  aaou_log_info((X));   \
  return 0;             \
} while (0)

const char *AGS_GetPluginName() {
	return "AGS AppOpenURL Plugin";
}

void aaou_log_info(const std::string& message){
    if(engine == nullptr) return;
    if(engine->version >= 29)
    {
        engine->Log(AGSLOG_LEVEL_INFO, "%s", message.c_str());
    } else {
        engine->PrintDebugConsole(message.c_str());
    }
}

size_t aaou_strnlen_s (const char* s, size_t n)
{
    const char* found = (const char*) memchr(s, '\0', n);
    return found ? (size_t)(found-s) : n;
}


int AGS_AppOpenURL(int iags_protocol, char const * iags_url_str)
{
    if(iags_url_str == nullptr || iags_url_str[0] == 0) {
        FAIL_LOG_AND_EXIT("AppOpenURL: empty URL received.");
    }

    if(aaou_strnlen_s(iags_url_str, MAX_URL_SIZE) == MAX_URL_SIZE) {
        FAIL_LOG_AND_EXIT("AppOpenURL: URL is too big.");
    }

    std::string url_str = iags_url_str;
    std::string rem_chars = " \t\n\r\n";
    for (char rem_char : rem_chars) {
        url_str.erase(std::remove(url_str.begin(), url_str.end(), rem_char), url_str.end());
    }

    if(url_str.empty()) {
        FAIL_LOG_AND_EXIT("AppOpenURL: URL was empty after clean up.");
    }
    if(url_str[0] == ':' || (url_str.rfind("://") != std::string::npos))  {
        FAIL_LOG_AND_EXIT("AppOpenURL: URL included protocol specifiers.");
    }

    auto proto = (AgsUrlProtocol) iags_protocol;
    std::string str_proto;
    switch (proto) {
        case eAgsUrlProt_https: str_proto = "https"; break;
        case eAgsUrlProt_http:  str_proto = "http";  break;
        default: str_proto = "https";
    }

    std::string url_to_open = str_proto+"://"+url_str;

    int return_val = SDL_OpenURL(url_to_open.c_str());
    if(return_val!=0) {
        FAIL_LOG_AND_EXIT("AppOpenURL: failed to launch url");
    }
    aaou_log_info("AppOpenURL: success launching url");
    return 1;
}

void AGS_EngineStartup(IAGSEngine *lpEngine)
{
  engine = lpEngine;

  // Make sure it's got the version with the features we need
  if (engine->version < 3)
  {
    engine->AbortGame ("Engine interface is too old, need newer version of AGS.");
  }

  engine->RegisterScriptFunction("AppOpenURL", (void *)(AGS_AppOpenURL));
}

void AGS_EngineShutdown() {}
int AGS_EngineOnEvent (int event, int data) { return 0; }
int AGS_PluginV2 () { return 1; }

// ****** END RUN TIME ********