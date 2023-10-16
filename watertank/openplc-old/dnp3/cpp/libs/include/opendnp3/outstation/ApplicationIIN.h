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
#ifndef OPENDNP3_APPLICATIONIIN_H
#define OPENDNP3_APPLICATIONIIN_H

#include <openpal/executor/UTCTimestamp.h>

#include <opendnp3/app/IINField.h>

namespace opendnp3
{

/**
	Some IIN bits are necessarily controlled by the outstation application,
	not the underlying protocol stack. This structure describes the state of
	the bits controllable by the application.
*/
class ApplicationIIN
{

public:

	ApplicationIIN();

	bool needTime;
	bool localControl;
	bool deviceTrouble;
	bool configCorrupt;

	IINField ToIIN() const;

};


}

#endif
