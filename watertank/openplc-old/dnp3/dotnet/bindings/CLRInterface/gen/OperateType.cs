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

namespace Automatak.DNP3.Interface
{
  /// <summary>
  /// Various ways that an outstation can receive a request to operate a BO or AO point
  /// </summary>
  public enum OperateType : byte
  {
    /// <summary>
    /// The outstation received a valid prior SELECT followed by OPERATE
    /// </summary>
    SelectBeforeOperate = 0x0,
    /// <summary>
    /// The outstation received a direct operate request
    /// </summary>
    DirectOperate = 0x1,
    /// <summary>
    /// The outstation received a direct operate no ack request
    /// </summary>
    DirectOperateNoAck = 0x2
  }
}
