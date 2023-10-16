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
#ifndef OPENPAL_DYNAMICARRAY_H
#define OPENPAL_DYNAMICARRAY_H

#include "ArrayView.h"

#include "openpal/Configure.h"
#include "openpal/util/Uncopyable.h"

#include <assert.h>
#include <cstdint>

namespace openpal
{

/**
* Template type for a dynamically allocated array
*/
template <class ValueType, class IndexType>
class Array : public HasSize<IndexType>//, private openpal::Uncopyable
{

public:

	Array(IndexType size) :
		HasSize<IndexType>(size),
		buffer(new ValueType[size]())
	{}

	Array() :
		HasSize<IndexType>(0),
		buffer(nullptr)
	{}

	Array(const Array& copy) :
		HasSize<IndexType>(copy.Size()),
		buffer(new ValueType[copy.Size()])
	{
		for(IndexType i = 0; i < copy.Size(); ++i) buffer[i] = copy.buffer[i];
	}

	ArrayView<ValueType, IndexType> ToView() const
	{
		return ArrayView<ValueType, IndexType>(buffer, this->size);
	}

	inline bool Contains(IndexType index) const
	{
		return index < this->size;
	}

	inline ValueType& operator[](IndexType index)
	{
		assert(index < this->size);
		return buffer[index];
	}

	const ValueType& operator[](IndexType index) const
	{
		assert(index < this->size);
		return buffer[index];
	}

	template <class Action>
	void foreach(const Action& action) const
	{
		for(IndexType i = 0; i < this->size; ++i) action(buffer[i]);
	}

	template <class Action>
	void foreach(const Action& action)
	{
		for(IndexType i = 0; i < this->size; ++i) action(buffer[i]);
	}

	template <class Action>
	void foreachIndex(const Action& action)
	{
		for(IndexType i = 0; i < this->size; ++i) action(buffer[i], i);
	}


	template <class Action>
	void foreachIndex(const Action& action) const
	{
		for(uint32_t i = 0; i < this->size; ++i) action(buffer[i], i);
	}

	virtual ~Array()
	{
		delete[] buffer;
	}

protected:

	ValueType* buffer;

private:

	Array& operator=(const Array&) = delete;
};

}

#endif
