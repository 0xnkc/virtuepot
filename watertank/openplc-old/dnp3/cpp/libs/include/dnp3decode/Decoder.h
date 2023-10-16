/*
 * Licensed to Green Energy Corp (www.greenenergycorp.com) under one or
 * more contributor license agreements. See the NOTICE file distributed
 * with this work for additional information regarding copyright ownership.
 * Green Energy Corp licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This project was forked on 01/01/2013 by Automatak, LLC and modifications
 * may have been made to this file. Automatak, LLC licenses these modifications
 * to you under the terms of the License.
 */
#ifndef OPENDNP3_DECODER_H
#define OPENDNP3_DECODER_H

#include <openpal/container/RSlice.h>
#include <openpal/logging/Logger.h>

#include "IDecoderCallbacks.h"

namespace opendnp3
{
class DecoderImpl;

// stand-alone DNP3 decoder
class Decoder
{
public:

	Decoder(IDecoderCallbacks& callbacks, openpal::Logger logger);
	~Decoder();

	void DecodeLPDU(const openpal::RSlice& data);
	void DecodeTPDU(const openpal::RSlice& data);
	void DecodeAPDU(const openpal::RSlice& data);



private:

	DecoderImpl* impl;
};


}

#endif
