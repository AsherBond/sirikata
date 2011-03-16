#include "JSFakeroot.hpp"

#include <sirikata/oh/Platform.hpp>

#include "../JSObjectScriptManager.hpp"
#include "../JSObjectScript.hpp"

#include "../JSSerializer.hpp"
#include "../JSPattern.hpp"
#include "../JSObjectStructs/JSContextStruct.hpp"
#include "JSFields.hpp"
#include "JSSystem.hpp"
#include "JSObjectsUtils.hpp"

#include <sirikata/core/util/SpaceObjectReference.hpp>


namespace Sirikata {
namespace JS {
namespace JSFakeroot {

v8::Handle<v8::Value> root_canSendMessage(const v8::Arguments& args)
{
    String errorMessage = "Error decoding the fakeroot object from root_canSendMessage.  ";
    JSFakerootStruct* jsfake  = JSFakerootStruct::decodeRootStruct(args.This(), errorMessage);

    if (jsfake == NULL)
        return v8::ThrowException( v8::Exception::Error(v8::String::New(errorMessage.c_str(), errorMessage.length())) );

    return jsfake->struct_canSendMessage();
}


v8::Handle<v8::Value> root_canRecvMessage(const v8::Arguments& args)
{
    String errorMessage = "Error decoding the fakeroot object from root_canRecvMessage.  ";
    JSFakerootStruct* jsfake  = JSFakerootStruct::decodeRootStruct(args.This(),errorMessage);

    if (jsfake == NULL)
        return v8::ThrowException( v8::Exception::Error(v8::String::New(errorMessage.c_str(), errorMessage.length())) );

    return jsfake->struct_canRecvMessage();    
}

v8::Handle<v8::Value> root_canImport(const v8::Arguments& args)
{
    String errorMessage = "Error decoding the fakeroot object from root_canImport.  ";
    JSFakerootStruct* jsfake  = JSFakerootStruct::decodeRootStruct(args.This(),errorMessage);

    if (jsfake == NULL)
        return v8::ThrowException( v8::Exception::Error(v8::String::New(errorMessage.c_str(), errorMessage.length())) );

    return jsfake->struct_canImport();
}


v8::Handle<v8::Value> root_canProx(const v8::Arguments& args)
{
    String errorMessage = "Error decoding the fakeroot object from root_canProx.  ";
    JSFakerootStruct* jsfake  = JSFakerootStruct::decodeRootStruct(args.This(),errorMessage);

    if (jsfake == NULL)
        return v8::ThrowException( v8::Exception::Error(v8::String::New(errorMessage.c_str(), errorMessage.length())) );

    return jsfake->struct_canProx();
}


v8::Handle<v8::Value> root_import(const v8::Arguments& args)
{
    if (args.Length() != 1)
        return v8::ThrowException( v8::Exception::Error(v8::String::New("Import only takes one parameter: the name of the file to import.")) );

    v8::Handle<v8::Value> filename = args[0];



    //decode the filename to import from.
    String strDecodeErrorMessage = "Error decoding string as first argument of root_import of jsfakeroot.  ";
    String native_filename; //string to decode to.
    bool decodeStrSuccessful = decodeString(args[0],native_filename,strDecodeErrorMessage);
    if (! decodeStrSuccessful)
        return v8::ThrowException( v8::Exception::Error(v8::String::New(strDecodeErrorMessage.c_str(), strDecodeErrorMessage.length())) );



    //decode the fakeroot object
    String errorMessage = "Error decoding the fakeroot object from root_import.  ";
    JSFakerootStruct* jsfake  = JSFakerootStruct::decodeRootStruct(args.This(),errorMessage);

    if (jsfake == NULL)
        return v8::ThrowException( v8::Exception::Error(v8::String::New(errorMessage.c_str(), errorMessage.length())) );

    return jsfake->struct_import(native_filename);
}

v8::Handle<v8::Value> root_getPosition(const v8::Arguments& args)
{
    String errorMessage = "Error decoding the fakeroot object from root_getPosition.  ";
    JSFakerootStruct* jsfake  = JSFakerootStruct::decodeRootStruct(args.This(),errorMessage);

    if (jsfake == NULL)
        return v8::ThrowException( v8::Exception::Error(v8::String::New(errorMessage.c_str(), errorMessage.length())) );

    return jsfake->struct_getPosition();
}

v8::Handle<v8::Value> root_print(const v8::Arguments& args)
{
    if (args.Length() != 1)
        return v8::ThrowException( v8::Exception::Error(v8::String::New("Error in root_print.  Requires exactly one argument: a string to print.")));
    
    String errorMessage = "Error decoding the fakeroot object from root_print.  ";
    JSFakerootStruct* jsfake  = JSFakerootStruct::decodeRootStruct(args.This(),errorMessage);

    if (jsfake == NULL)
        return v8::ThrowException( v8::Exception::Error(v8::String::New(errorMessage.c_str(), errorMessage.length())) );


    v8::String::Utf8Value str(args[0]);
    String toPrint( ToCString(str));
    
    return jsfake->struct_print(toPrint);    
}


v8::Handle<v8::Value> root_sendHome(const v8::Arguments& args)
{
    if (args.Length() != 1)
        return v8::ThrowException( v8::Exception::Error(v8::String::New("Error in root_sendHome.  Requires exactly one argument: an object to send.")));

    if (! args[0]->IsObject())
        return v8::ThrowException( v8::Exception::Error(v8::String::New("Error in root_sendHome.  Requires argument to be an object.")));

    //decode string argument
    v8::Handle<v8::Value> messageBody = args[0];
    if(!messageBody->IsObject())
        return v8::ThrowException(v8::Exception::Error(v8::String::New("Message should be an object in root_sendHome.")) );

    //serialize the object to send
    Local<v8::Object> v8Object = messageBody->ToObject();
    String serialized_message = JSSerializer::serializeObject(v8Object);
    
    
    String errorMessage = "Error decoding the fakeroot object from root_print.  ";
    JSFakerootStruct* jsfake  = JSFakerootStruct::decodeRootStruct(args.This(),errorMessage);

    if (jsfake == NULL)
        return v8::ThrowException( v8::Exception::Error(v8::String::New(errorMessage.c_str(), errorMessage.length())) );

    return jsfake->struct_sendHome(serialized_message);
}



v8::Handle<v8::Value> root_timeout(const v8::Arguments& args)
{
    //just returns the ScriptTimeout function
    String errorMessage      =  "Error decoding fakeroot in root_timeout of JSFakeroot.cpp.  ";
    JSFakerootStruct* jsfake = JSFakerootStruct::decodeRootStruct(args.This(),errorMessage);

    if (jsfake == NULL)
        return v8::ThrowException(v8::Exception::Error(v8::String::New(errorMessage.c_str(),errorMessage.length())));
    
    return JSSystem::ScriptTimeoutContext(args, jsfake->associatedContext);
}


v8::Handle<v8::Value> root_registerHandler(const v8::Arguments& args)
{
    std::cout<<"\n\nIn JSFakeroot.cpp.  Haven't finished root_registerHandler.\n\n";
    assert(false);
    return v8::Undefined();
}

v8::Handle<v8::Value> root_toString(const v8::Arguments& args)
{
    //note to string probably should not serialize fakeroot object, but instead
    //should just be a keyword for you should do this on your end
    std::cout<<"\n\nIn JSFakeroot.cpp.  Haven't finished root_toString.\n\n";
    assert(false);
    return v8::Undefined();
}






}//end jsfakeroot namespace
}//end js namespace
}//end sirikata


