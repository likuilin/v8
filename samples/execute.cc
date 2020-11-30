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
    if (argc < 2) {
        printf("Usage: ./execute stuff.jsc\n");
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
            // Read data
            FILE *file = fopen(argv[1], "r+");
            if (!file) {
                printf("Open file failed\n");
                return -1;
            }
            fseek(file, 0, SEEK_END);
            int length = ftell(file);
            fseek(file, 0, SEEK_SET);
            uint8_t *data = (uint8_t *) malloc(length);
            fread(data, 1, length, file);
            fclose(file);

            // Create cache
            v8::ScriptCompiler::CachedData *cache =
                new v8::ScriptCompiler::CachedData(data, length);

            // Make into source with empty actual source string
            v8::ScriptCompiler::Source source_(v8::String::Empty(isolate), cache);
            
            // Compile the source using consume code cache
            v8::Local<v8::Script> script = v8::ScriptCompiler::Compile(context, &source_,
                v8::ScriptCompiler::kConsumeCodeCache).ToLocalChecked();

            // Run the script to get the result.
            v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

            // Convert the result to an UTF8 string and print it.
            v8::String::Utf8Value utf8(isolate, result);
            printf("%s\n", *utf8);

            // Free data
            free(data);
        }
    }

    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    return 0;
}
