#include "approve.hpp"
#include <mcp/common/utility.hpp>
#include <libdevcore/CommonIO.h>
#include <boost/endian/conversion.hpp>
#include <mcp/common/common.hpp>
#include <mcp/common/log.hpp>
#include "config.hpp"
#include <vector>

Address const& mcp::approve::sender() const
{
	if (!m_sender.is_initialized())
	{
		auto p = recover(m_vrs, sha3(WithoutSignature));
		if (!p)
			BOOST_THROW_EXCEPTION(InvalidSignature());
		m_sender = right160(dev::sha3(bytesConstRef(p.data(), sizeof(p))));
		m_publicCompressed = dev::toPublicCompressed(p);
	}
	return *m_sender;
}

SignatureStruct const& mcp::approve::signature() const
{
	return m_vrs;
}

u256 mcp::approve::rawV() const
{
	int const vOffset = m_chainId * 2 + 35;
	return m_vrs.v + vOffset;
}

void mcp::approve::sign(Secret const& priv)
{
	auto sig = dev::sign(priv, sha3(WithoutSignature));
	SignatureStruct sigStruct = *(SignatureStruct const*)&sig;
	if (sigStruct.isValid())
		m_vrs = sigStruct;
	else
		BOOST_THROW_EXCEPTION(InvalidSignature() << errinfo_comment("signatue invalid"));
}

static const u256 c_secp256k1n("115792089237316195423570985008687907852837564279074904382605163141518161494337");

void mcp::approve::checkLowS() const
{
	if (m_vrs.s > c_secp256k1n / 2)
		BOOST_THROW_EXCEPTION(InvalidSignature());
}

void mcp::approve::checkChainId(uint64_t _chainId) const
{
	if (m_chainId != _chainId)
		BOOST_THROW_EXCEPTION(InvalidSignature());
}

h256 mcp::approve::sha3(IncludeSignature _sig) const
{
	if (_sig == WithSignature && m_hashWith)
		return m_hashWith;

	RLPStream s;
	streamRLP(s, _sig);

	auto ret = dev::sha3(s.out());
	if (_sig == WithSignature)
		m_hashWith = ret;
	return ret;
}

mcp::witnessElectionApprove::witnessElectionApprove(Epoch const & _epoch, h648 const & _proof, Secret const& s) :
	m_epoch(_epoch), m_proof(_proof)
{
	if (s)
		sign(s);
}

mcp::witnessElectionApprove::witnessElectionApprove(dev::RLP const & rlp, CheckTransaction _checkSig)
{
    try
	{
		if (!rlp.isList())
			BOOST_THROW_EXCEPTION(InvalidTransactionFormat() << errinfo_comment("approve RLP must be a list"));
		ApproveType type = static_cast<ApproveType>(rlp[0].toInt<uint64_t>());
		assert(type == WitnessElection);

		if (rlp.itemCount() != 6)
		BOOST_THROW_EXCEPTION(InvalidTransactionFormat() << errinfo_comment("too many or to low fields in the transaction RLP"));

		m_epoch = rlp[1].toInt<uint64_t>();
        m_proof = (h648)rlp[2];
		u256 const v = rlp[3].toInt<u256>();
		h256 const r = rlp[4].toInt<u256>();
		h256 const s = rlp[5].toInt<u256>();

		if (v > 36)
		{
			auto const chainId = (v - 35) / 2;
			if (chainId > std::numeric_limits<uint64_t>::max())
				BOOST_THROW_EXCEPTION(InvalidSignature());
			m_chainId = static_cast<uint64_t>(chainId);
		}
		// not support non-replay protected approves
		else if (v != 27 && v != 28)
			BOOST_THROW_EXCEPTION(InvalidSignature());

		auto const recoveryID = byte(v - (u256(m_chainId) *2 + 35));
		m_vrs = SignatureStruct{ r, s, recoveryID };

		///check signature valid if broadcast
		if (_checkSig >= CheckTransaction::Cheap && !m_vrs.isValid())
			BOOST_THROW_EXCEPTION(InvalidSignature());

		if (_checkSig == CheckTransaction::Everything)
			m_sender = sender();
}
	catch (Exception& _e)
	{
		_e << errinfo_name("invalid approve format: " + toString(rlp) + " RLP: " + toHex(rlp.data()));
		throw;
	}
}

void mcp::witnessElectionApprove::streamRLP(RLPStream& s, IncludeSignature sig) const
{
	s.appendList(6);
	s << type() << m_epoch << m_proof;
	if (sig == IncludeSignature::WithSignature) //rlp for p2p and storage
	{
		s << rawV() << (u256)m_vrs.r << (u256)m_vrs.s;
	}
	else  ///rlp for hash and verify sinature
		s << m_chainId << 0 << 0;
}

void mcp::witnessElectionApprove::checkEpoch(uint64_t _epoch) const
{
	if (m_epoch != _epoch)
	{
		LOG(g_log.error) << "[checkEpoch] m_epoch = " << m_epoch << " _epoch = " <<_epoch;
		BOOST_THROW_EXCEPTION(InvalidSignature());
	}
}

void mcp::witnessElectionApprove::vrf_verify(mcp::block_hash const& msg) const
{
	sender();
	if(!dev::verify(m_outputs, m_proof, m_publicCompressed, msg))
	{
		//LOG(g_log.debug) << "[vrf_verify] secp256k1_vrf_verify fail ";
		BOOST_THROW_EXCEPTION(InvalidSignature());
	}
	//LOG(g_log.debug) << "[vrf_verify] secp256k1_vrf_verify ok";
}


mcp::denMiningApprove::denMiningApprove(DenMiningSkeleton _d, Secret const& _s) : 
	m_mci(_d.mci), m_hash(_d.hash)
{
	if (_s)
		sign(_s);
}

mcp::denMiningApprove::denMiningApprove(dev::RLP const & rlp, CheckTransaction _checkSig)
{
    try
	{
		if (!rlp.isList())
			BOOST_THROW_EXCEPTION(InvalidTransactionFormat() << errinfo_comment("approve RLP must be a list"));
		ApproveType type = static_cast<ApproveType>(rlp[0].toInt<uint64_t>());
		assert(type == DENMiningPing);

		if (rlp.itemCount() != 6)
		BOOST_THROW_EXCEPTION(InvalidTransactionFormat() << errinfo_comment("too many or to low fields in the transaction RLP"));

		m_mci = rlp[1].toInt<uint64_t>();
		m_hash = (block_hash)rlp[2];

		u256 const v = rlp[3].toInt<u256>();
		h256 const r = rlp[4].toInt<u256>();
		h256 const s = rlp[5].toInt<u256>();

		if (v > 36)
		{
			auto const chainId = (v - 35) / 2;
			if (chainId > std::numeric_limits<uint64_t>::max())
				BOOST_THROW_EXCEPTION(InvalidSignature());
			m_chainId = static_cast<uint64_t>(chainId);
		}
		// not support non-replay protected approves
		else if (v != 27 && v != 28)
			BOOST_THROW_EXCEPTION(InvalidSignature());

		auto const recoveryID = byte(v - (u256(m_chainId) *2 + 35));
		m_vrs = SignatureStruct{ r, s, recoveryID };

		///check signature valid if broadcast
		if (_checkSig >= CheckTransaction::Cheap && !m_vrs.isValid())
			BOOST_THROW_EXCEPTION(InvalidSignature());

		if (_checkSig == CheckTransaction::Everything)
			m_sender = sender();
	}
	catch (Exception& _e)
	{
		_e << errinfo_name("invalid approve format: " + toString(rlp) + " RLP: " + toHex(rlp.data()));
		throw;
	}
}

void mcp::denMiningApprove::streamRLP(RLPStream& s, IncludeSignature sig) const
{
	s.appendList(6);
	s << type() << m_mci << m_hash;
	if (sig == IncludeSignature::WithSignature) //rlp for p2p and storage
	{
		s << rawV() << (u256)m_vrs.r << (u256)m_vrs.s;
	}
	else  ///rlp for hash and verify sinature
		s << m_chainId << 0 << 0;
}

std::shared_ptr<mcp::approve> mcp::approveFromRLP(dev::RLP const & r, mcp::CheckTransaction _checkSig)
{
	assert(r.isList());
	std::shared_ptr<mcp::approve> result = nullptr;
	mcp::ApproveType type = static_cast<mcp::ApproveType>(r[0].toInt<uint64_t>());
	switch (type)
	{
	case mcp::WitnessElection:
		{
			std::shared_ptr<mcp::witnessElectionApprove> a = std::make_shared<mcp::witnessElectionApprove>(r, mcp::CheckTransaction::None);
			result = a;
			break;
		}
	case mcp::DENMiningPing:
		{
			std::shared_ptr<mcp::denMiningApprove> a = std::make_shared<mcp::denMiningApprove>(r, mcp::CheckTransaction::None);
			result = a;
			if(result){
				LOG(g_log.debug) << "[approveFromRLP] result not null "<< result->sender().hex();
			}
			else{
				LOG(g_log.debug) << "[approveFromRLP] is null";
			}
			break;
		}
	default:
		assert(false);
		break;
	}
	return result;
}