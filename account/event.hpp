// Olympus: mcp C++ client, tools and libraries.
// Licensed under the GNU General Public License, Version 3.

/// @file
/// This file defines contract abi interface type.
#pragma once
#include "argument.hpp"

namespace dev
{
	/// Event is an event potentially triggered by the EVM's LOG mechanism. The Event
	/// holds type information (inputs) about the yielded output. Anonymous events
	/// don't get the signature canonical representation as the first LOG topic.
	class Event
	{
	public:
		Event() {};
		Event(std::string const& _Name, std::string const& _RawName, bool const& _Anonymous, Arguments const& _Inputs
			//, std::string const& _str
			, std::string const& _Sig, dev::h256 const& _ID
		):
			Name(_Name),
			RawName(_RawName),
			Anonymous(_Anonymous),
			Inputs(_Inputs),
			// str(_str),
			Sig(_Sig),
			ID(_ID)
		{}

		/// Name is the event name used for internal representation. It's derived from
		/// the raw name and a suffix will be added in the case of event overloading.
		///
		/// e.g.
		/// These are two events that have the same name:
		/// * foo(int,int)
		/// * foo(uint,uint)
		/// The event name of the first one will be resolved as foo while the second one
		/// will be resolved as foo0.
		std::string Name;

		/// RawName is the raw event name parsed from ABI.
		std::string RawName;
		bool Anonymous;
		Arguments Inputs;
		std::string str;

		/// Sig contains the string signature according to the ABI spec.
		/// e.g.	 event foo(uint32 a, int b) = "foo(uint32,int256)"
		/// Please note that "int" is substitute for its canonical representation "int256"
		std::string Sig;

		/// ID returns the canonical representation of the event's signature used by the
		/// abi definition to identify event names and types.
		dev::h256 ID;
	};
	
	// NewEvent creates a new Event.
	// It sanitizes the input arguments to remove unnamed arguments.
	// It also precomputes the id, signature and string representation
	// of the event.
	Event NewEvent(std::string const& _name, std::string const& _RawName, bool  const& _anonymous, Arguments const& _Inputs);
}

