#include "event.hpp"
#include <libdevcore/SHA3.h>

namespace dev
{
    Event NewEvent(std::string const& _name, std::string const& _RawName, bool const& _anonymous, Arguments const& _Inputs)
    {
		std::vector<std::string> types(_Inputs.size()), inputNames(_Inputs.size());
		for (size_t i = 0; i < _Inputs.size(); i++)
		{
			inputNames[i] = boost::str(boost::format("%1% %2%") % _Inputs[i].Typ->stringKind % _Inputs[i].Name);
			types[i] = _Inputs[i].Typ->stringKind;
		}

		/// calculate the signature and method id. Note only function
        /// has meaningful signature and id.
		std::string sig;
		dev::h256 id;
        sig = _RawName + "(" + boost::algorithm::join(types, ",") + ")";
        id = dev::sha3(sig);

        return Event(_name, _RawName, _anonymous, _Inputs, sig, id);
    }
}