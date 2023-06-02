#pragma once

#include <libdevcore/RLP.h>
#include <libdevcore/Address.h>
#include <mcp/common/EVMSchedule.h>
#include <libdevcrypto/Common.h>
#include <boost/optional.hpp>
#include "transaction.hpp"
#include <secp256k1-vrf.h>

using namespace dev;
namespace mcp
{
	struct DenMiningSkeleton
	{
		Address from;
		uint64_t mci;
		block_hash hash;
	};

	struct den_mining_ping{
		uint64_t mci;
		block_hash hash;
	};
	
	enum ApproveType {WitnessElection=0, DENMiningPing=1};
	
	class approve
	{
	public:

		approve() : m_chainId(mcp::chain_id) 
		{}

		/// @returns sender of the transaction from the signature (and hash).
		/// @throws TransactionIsUnsigned if signature was not initialized
		Address const& sender() const;

		/// @throws TransactionIsUnsigned if signature was not initialized
		/// @throws InvalidSValue if the signature has an invalid S value.
		void checkLowS() const;

		/// @throws InvalidSignature if the transaction is replay protected
		/// and chain id is not equal to @a _chainId
		void checkChainId(uint64_t _chainId) const;

		/// @returns the SHA3 hash of the RLP serialisation of this approve.
		/// queue used WithSignature hash inlcude sinature,if sinature error,can resend it again with the correct signature
		h256 sha3(IncludeSignature _sig = WithSignature) const;

		//test
		uint64_t chainID() const { return m_chainId; }

		/// @returns the signature of the transaction (the signature has the sender encoded in it)
		/// @throws TransactionIsUnsigned if signature was not initialized
		SignatureStruct const& signature() const;

		/// @returns v value of the transaction (has chainID and recoveryID encoded in it)
		/// @throws TransactionIsUnsigned if signature was not initialized
		u256 rawV() const;

		void sign(Secret const& _priv);			///< Sign the transaction.
		
		virtual ApproveType type() const = 0;
		virtual void streamRLP(dev::RLPStream& s, IncludeSignature sig = WithSignature) const = 0;

		SignatureStruct m_vrs;	///< The signature of the approve. Encodes the sender.
		uint64_t m_chainId;
		mutable h256 m_hashWith;			///< Cached hash of approve with signature.
		mutable dev::PublicCompressed m_publicCompressed;
		mutable boost::optional<Address> m_sender;  ///< Cached sender, determined from signature.
	};

	class witnessElectionApprove : public approve
	{
	public:

		/// Constructs a transaction from a transaction skeleton & optional secret.
		witnessElectionApprove(Epoch const & _epoch, h648 const & _proof, Secret const& _s);

		/// Constructs a approve from the given RLP.
		witnessElectionApprove(dev::RLP const & r, CheckTransaction _checkSig);

		/// Constructs a transaction from the given RLP.
		explicit witnessElectionApprove(bytesConstRef _rlp, CheckTransaction _checkSig) : witnessElectionApprove(RLP(_rlp), _checkSig) {}

		/// Constructs a Transaction from the given RLP.
		explicit witnessElectionApprove(bytes const& _rlp, CheckTransaction _checkSig) : witnessElectionApprove(&_rlp, _checkSig) {}

		ApproveType type() const { return ApproveType::WitnessElection; }

		void checkEpoch(uint64_t _epoch) const;

		/// Serialises this approve to an RLPStream.
		/// @throws TransactionIsUnsigned if including signature was requested but it was not initialized
		void streamRLP(dev::RLPStream& s, IncludeSignature sig = WithSignature) const;

		/// @returns the RLP serialisation of this transaction.
		bytes rlp(IncludeSignature _sig = WithSignature) const { RLPStream s; streamRLP(s, _sig); return s.out(); }

		void vrf_verify(mcp::block_hash const& msg) const;
		
		h256 outputs() { return m_outputs; }
		Epoch epoch() const { return m_epoch; }
		h648 proof() const { return m_proof; }

	private:
		Epoch m_epoch;
		h648 m_proof;
		mutable h256 m_outputs;			    ///< Cached output of proof.
	};

	class denMiningApprove : public approve
	{
	public:
		denMiningApprove(DenMiningSkeleton _d, Secret const& _s);

		/// Constructs a approve from the given RLP.
		denMiningApprove(dev::RLP const & r, CheckTransaction _checkSig);

		/// Constructs a transaction from the given RLP.
		explicit denMiningApprove(bytesConstRef _rlp, CheckTransaction _checkSig) : denMiningApprove(RLP(_rlp), _checkSig) {}

		/// Constructs a Transaction from the given RLP.
		explicit denMiningApprove(bytes const& _rlp, CheckTransaction _checkSig) : denMiningApprove(&_rlp, _checkSig) {}

		ApproveType type() const { return ApproveType::DENMiningPing; }

		/// Serialises this approve to an RLPStream.
		/// @throws TransactionIsUnsigned if including signature was requested but it was not initialized
		void streamRLP(dev::RLPStream& s, IncludeSignature sig = WithSignature) const;

		/// @returns the RLP serialisation of this transaction.
		bytes rlp(IncludeSignature _sig = WithSignature) const { RLPStream s; streamRLP(s, _sig); return s.out(); }

		uint64_t mci() const { return m_mci; }
		block_hash hash() const { return m_hash; }

	private:
		uint64_t m_mci;
		block_hash m_hash;
	};

	std::shared_ptr<approve> approveFromRLP(dev::RLP const & r, CheckTransaction _checkSig);
}
