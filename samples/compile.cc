// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

#include <fstream>
#include <streambuf>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: ./compile input.js output.jsc\n");
        return -1;
    }
    // Initialize V8.
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    // Create a new Isolate and make it the current one.
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
            v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);

        // Create a stack-allocated handle scope.
        v8::HandleScope handle_scope(isolate);

        // Create a new context.
        v8::Local<v8::Context> context = v8::Context::New(isolate);

        // Enter the context for compiling and running the hello world script.
        v8::Context::Scope context_scope(context);

        {
            // Create a string containing the JavaScript source code from the file
            std::ifstream in_file(argv[1]);
            if (in_file.fail()) {
                printf("File read\n");
                return -1;
            }
            std::string str((std::istreambuf_iterator<char>(in_file)), std::istreambuf_iterator<char>());

            // Make the string into a JS string
            v8::Local<v8::String> source_str =
                v8::String::NewFromUtf8(isolate, str.c_str(), v8::NewStringType::kNormal).ToLocalChecked();

            // Compile the source code.
            v8::Local<v8::Script> script = v8::Script::Compile(context, source_str).ToLocalChecked();

            // Get unbound script
            v8::Local<v8::UnboundScript> unbound_script = script->GetUnboundScript();

            // Run the script so we get all the reached functions
            script->Run(context).ToLocalChecked();

            // Create code cache
            v8::ScriptCompiler::CachedData *cached_data = v8::ScriptCompiler::CreateCodeCache(unbound_script);
            if (cached_data->rejected) {
                // Rejected, whatever that means
                printf("Compilation rejected");
                return -1;
            }

            // Copy into output
            FILE *out_file = fopen(argv[2], "w");
            fwrite(cached_data->data, 1, cached_data->length, out_file);
            fclose(out_file);
        }
    }

    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    return 0;
}
