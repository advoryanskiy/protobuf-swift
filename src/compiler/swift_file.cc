// Protocol Buffers for Swift
//
// Copyright 2014 Alexey Khohklov(AlexeyXo).
// Copyright 2008 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "swift_file.h"

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/stubs/strutil.h>

#include <sstream>

#include "swift_enum.h"
#include "swift_oneof.h"
#include "swift_extension.h"
#include "swift_helpers.h"
#include "swift_message.h"

namespace google { namespace protobuf { namespace compiler {namespace swift {

  FileGenerator::FileGenerator(const FileDescriptor* file)
    : file_(file),
    classname_(FileClassName(file)) {
  }


  FileGenerator::~FileGenerator() {
  }

  void DetermineDependenciesWorker(set<string>* dependencies, set<string>* seen_files, const FileDescriptor* file) {
    if (seen_files->find(file->name()) != seen_files->end()) {
      // don't infinitely recurse
      return;
    }

    seen_files->insert(file->name());

    for (int i = 0; i < file->dependency_count(); i++) {
      DetermineDependenciesWorker(dependencies, seen_files, file->dependency(i));
    }
    for (int i = 0; i < file->message_type_count(); i++) {
      MessageGenerator(file->message_type(i)).DetermineDependencies(dependencies);
    }
  }


  void FileGenerator::DetermineDependencies(set<string>* dependencies) {
    set<string> seen_files;
    DetermineDependenciesWorker(dependencies, &seen_files, file_);
  }


  void FileGenerator::GenerateSource(io::Printer* printer) {
    FileGenerator file_generator(file_);
    printer->Print(
      "// Generated by the protocol buffer compiler.  DO NOT EDIT!\n\n"
      "import Foundation\n"
      "import ProtocolBuffers\n\n");

    printer->Print(
      "private class $classname$ {\n",
      "classname", classname_);
      
      

    for (int i = 0; i < file_->extension_count(); i++) {
      ExtensionGenerator(classname_, file_->extension(i)).GenerateFieldsSource(printer);
    }

    for (int i = 0; i < file_->message_type_count(); i++) {
      MessageGenerator(file_->message_type(i)).GenerateStaticVariablesSource(printer);
    }

      //TODO
    printer->Print("var extensionRegistry:ExtensionRegistry\n");
    printer->Print(
//      "class func extensionRegistry() -> ExtensionRegistry {\n"
//      "  return ExtensionRegistry()\n"
//      "}\n"
      "\n"
      "init() {\n"
      "",
      "classname", classname_);

    
    

    for (int i = 0; i < file_->extension_count(); i++) {
      ExtensionGenerator(classname_, file_->extension(i)).GenerateInitializationSource(printer);
    }

    for (int i = 0; i < file_->message_type_count(); i++) {
      MessageGenerator(file_->message_type(i)).GenerateStaticVariablesInitialization(printer);
    }

    printer->Print(
      "extensionRegistry = ExtensionRegistry()\n"
      "registerAllExtensions(extensionRegistry)\n");

    for (int i = 0; i < file_->dependency_count(); i++) {
      printer->Print(
        "$dependency$.registerAllExtensions(registry)\n",
        "dependency", FileClassName(file_->dependency(i)));
    }
    

    printer->Print(
      ""
      "}\n");

    

    // -----------------------------------------------------------------

    printer->Print(
      "func registerAllExtensions(registry:ExtensionRegistry) {\n");
    

    for (int i = 0; i < file_->extension_count(); i++) {
      ExtensionGenerator(classname_, file_->extension(i))
        .GenerateRegistrationSource(printer);
    }

    for (int i = 0; i < file_->message_type_count(); i++) {
      MessageGenerator(file_->message_type(i))
        .GenerateExtensionRegistrationSource(printer);
    }

    
    printer->Print(
      "}\n");

    // -----------------------------------------------------------------

    for (int i = 0; i < file_->extension_count(); i++) {
      ExtensionGenerator(classname_, file_->extension(i)).GenerateMembersSource(printer);
    }

    printer->Print(
      "}\n\n");
    
    

    for (int i = 0; i < file_->enum_type_count(); i++) {
        EnumGenerator(file_->enum_type(i)).GenerateSource(printer);
    }
    
    for (int i = 0; i < file_->message_type_count(); i++) {
       
        for (int j = 0; j < file_->message_type(i)->nested_type_count(); j++) {
            MessageGenerator(file_->message_type(i)->nested_type(j)).GenerateMessageIsEqualSource(printer);
        }
        MessageGenerator(file_->message_type(i)).GenerateMessageIsEqualSource(printer);
    }
      
    for (int i = 0; i < file_->message_type_count(); i++) {
        MessageGenerator(file_->message_type(i)).GenerateSource(printer);
    }
  
    //Extensions NSData
    printer->Print("//Class extensions: NSData\n\n\n");
      
    for (int i = 0; i < file_->message_type_count(); i++) {
      
        for (int j = 0; j < file_->message_type(i)->nested_type_count(); j++) {
            MessageGenerator(file_->message_type(i)->nested_type(j)).GenerateParseFromExtensionMethodsSource(printer);
        }
        MessageGenerator(file_->message_type(i)).GenerateParseFromExtensionMethodsSource(printer);
    }
      
    printer->Print(
      "\n"
      "// @@protoc_insertion_point(global_scope)\n");
  }
}  // namespace swift
}  // namespace compiler
}  // namespace protobuf
}  // namespace google