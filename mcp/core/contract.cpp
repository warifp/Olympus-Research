#include "contract.hpp"

namespace mcp
{
	DENContractCaller NewDENContractCaller(dev::ContractCaller const& _caller)
	{
		auto parsed = dev::JSON(DENContractABI);
		return DENContractCaller(dev::NewBoundContract(DENManagerAddress, parsed, _caller));
	}

	DENContractCaller DENCaller;
	
	Transaction InitDenContractTransaction()
	{
		TransactionSkeleton ts;
		ts.from = DENManagerAddress;
		ts.data = DENContractByteCode;
		ts.gasPrice = 10000000;
		//ts.gas = mcp::tx_max_gas;
		ts.gas = 1215903;
		ts.nonce = 2;///genesis account used 2
		Transaction _t(ts);
		_t.setSignature(h256("3e68e15c38d9903d0bca9b8b642f2ee6093ac95639f670b08da51a33cbeff14b"), h256("01ac3bedf905bccca62e3da33beba47e34d83e5a76dd587cd7c7b1010cfbc430"), 19435);
		return _t;
	}
	
	///for den caller
	dev::bytes DENContractCaller::isMiner(dev::Address minerAddress)
	{
		std::string method = "isMiner";
		return contract.Pack(method, minerAddress);
	}
	
	bool DENContractCaller::getIsMiner(dev::bytes& data)
	{
		bool ret;
		contract.Unpack("isMiner", data, ret);
		return ret;
	}


	///for main caller
	dev::bytes MainContractCaller::BatchTransfer(std::map<dev::Address, u256> const& _v)
	{
		std::string method = "batchTransfer";
		std::vector<dev::Address> address;
		std::vector<u256> values;
		for (auto it : _v)
		{
			address.push_back(it.first);
			values.push_back(it.second);
		}
		return contract.Pack(method, address, values);
	}

	Transaction InitMainContractTransaction()
	{
		TransactionSkeleton ts;
		ts.from = MainCallcAddress;
		ts.data = MainContractByteCode;
		ts.gasPrice = 10000000;
		//ts.gas = mcp::tx_max_gas;
		ts.gas = 1215903;
		ts.nonce = 1;///genesis account used 1
		Transaction _t(ts);
		_t.setSignature(h256("e3eb5af167c47deef60aee47b18ba33e442a62752735b99d290dffcae01c007c"), h256("063a7e367c5451e6c513dff4e719a6ee4784da2ee6dc654c2fe15f3415120f0f"), 19436);
		return _t;
	}

	MainContractCaller NewMainContractCaller(dev::ContractCaller const& _caller)
	{
		auto parsed = dev::JSON(MainContractABI);
		return MainContractCaller(dev::NewBoundContract(MainContractAddress, parsed, _caller));
	}
	MainContractCaller MainCaller;
}
