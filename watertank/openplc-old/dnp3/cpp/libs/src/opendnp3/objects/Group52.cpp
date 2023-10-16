//
//  _   _         ______    _ _ _   _             _ _ _
// | \ | |       |  ____|  | (_) | (_)           | | | |
// |  \| | ___   | |__   __| |_| |_ _ _ __   __ _| | | |
// | . ` |/ _ \  |  __| / _` | | __| | '_ \ / _` | | | |
// | |\  | (_) | | |___| (_| | | |_| | | | | (_| |_|_|_|
// |_| \_|\___/  |______\__,_|_|\__|_|_| |_|\__, (_|_|_)
//                                           __/ |
//                                          |___/
// 
// This file is auto-generated. Do not edit manually
// 
// Copyright 2013 Automatak LLC
// 
// Automatak LLC (www.automatak.com) licenses this file
// to you under the the Apache License Version 2.0 (the "License"):
// 
// http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Group52.h"

#include <openpal/serialization/Format.h>
#include <openpal/serialization/Parse.h>

using namespace openpal;

namespace opendnp3 {

// ------- Group52Var1 -------

Group52Var1::Group52Var1() : time(0)
{}

bool Group52Var1::Read(RSlice& buffer, Group52Var1& output)
{
  return Parse::Many(buffer, output.time);
}

bool Group52Var1::Write(const Group52Var1& arg, openpal::WSlice& buffer)
{
  return Format::Many(buffer, arg.time);
}

// ------- Group52Var2 -------

Group52Var2::Group52Var2() : time(0)
{}

bool Group52Var2::Read(RSlice& buffer, Group52Var2& output)
{
  return Parse::Many(buffer, output.time);
}

bool Group52Var2::Write(const Group52Var2& arg, openpal::WSlice& buffer)
{
  return Format::Many(buffer, arg.time);
}


}
