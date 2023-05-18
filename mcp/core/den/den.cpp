#include "den.hpp"
#include <algorithm>
#include <map>
#include <mcp/core/block_store.hpp>
#include <mcp/core/approve.hpp>
#include "mcp/rpc/jsonHelper.hpp"
#include "mcp/node/message.hpp"
#include "account/abi.hpp"

const uint8_t den_except_frozen_len = 6;
std::shared_ptr<mcp::den> mcp::g_den;
const std::string DENContractABI ="[{\
\"anonymous\": false,\
\"inputs\": [\
    {\
        \"indexed\": false,\
        \"internalType\": \"address\",\
        \"name\": \"minerAddress\",\
        \"type\": \"address\"\
    },\
    {\
        \"indexed\": false,\
        \"internalType\": \"uint256\",\
        \"name\": \"stakeAmount\",\
        \"type\": \"uint256\"\
    }\
],\
\"name\": \"Stake\",\
\"type\": \"event\"\
},\
{\
    \"anonymous\": false,\
    \"inputs\": [\
    {\
        \"indexed\": false,\
        \"internalType\": \"address\",\
        \"name\": \"minerAddress\",\
        \"type\": \"address\"\
    }\
    ],\
    \"name\": \"AddMiner\",\
    \"type\": \"event\"\
}]";

mcp::den::den(mcp::block_store& store_a) :
    m_store(store_a)
{
    // den_unit u;
    // m_den_units.emplace(jsToAddress("0x1144B522F45265C2DFDBAEE8E324719E63A1694C"), u);
    // m_den_units.emplace(jsToAddress("0xBAC0b1DD15093De9bBEb8f2E940eb0872A4E0bCD"), u);
    // m_den_units.emplace(jsToAddress("0x740233e47e13a30B650f3E5Bf59DCcc8Fd16B373"), u);
    m_abi = dev::JSON(DENContractABI);
}

void mcp::den::init(mcp::db::db_transaction & transaction_a)
{
    LOG(m_log.info) << "[den::init] in";
    m_store.den_witelist_get(transaction_a, m_den_witelist);
    for(auto addr : m_den_witelist){
        LOG(m_log.info) << "[den::init] witelist:" << addr.hexPrefixed();
    }
    // for(std::unordered_map<dev::Address, den_unit>::iterator it=m_den_units.begin(); it!=m_den_units.end(); it++){
    //     if(!m_store.den_rewards_get(transaction_a, it->first, it->second)){
    //         LOG(m_log.info) << "[den::init] den_rewards get ok. " << it->first.hexPrefixed();
    //     }
    //     else{
    //         LOG(m_log.info) << "[den::init] den_rewards not get. " << it->first.hexPrefixed();
    //         it->second.last_calc_day = mcp::seconds_since_epoch()/mcp::den_reward_period_day;
    //         it->second.last_handle_ping_time = mcp::seconds_since_epoch();
    //         m_store.den_rewards_put(transaction_a, it->first, it->second);
    //     }
    //     LOG(m_log.info) << "[den::init] last_calc_day=" << it->second.last_calc_day << " last_handle_ping_time=" << it->second.last_handle_ping_time;
    // }
    LOG(m_log.info) << "[den::init] ok. ";
}

void mcp::den::set_max_stake(const dev::u256 &v)
{
    m_param.max_stake = v;
}

void mcp::den::set_max_reward_perday(const dev::u256 &v)
{
    m_param.max_reward_perday = v;
}

void mcp::den::witelist_add(const dev::Address &addr)
{

}

void mcp::den::witelist_remove(const dev::Address &addr)
{

}

void mcp::den::stake(const dev::Address &addr, dev::u256 &v)
{

}

void mcp::den::unstake(const dev::Address &addr, dev::u256 &v)
{

}

void mcp::den::set_cur_time(const uint64_t &time)
{
    
}

void mcp::den::set_mc_block_time(const uint64_t &time, const block_hash &h)
{
    //m_time_block[time] = h;
}

void mcp::den::handle_event_setparam(mcp::db::db_transaction & transaction_a, const log_entry &log_a)
{
    m_abi.UnpackEvent("SetParam", log_a.data, m_param.max_reward_perday);
    LOG(m_log.info) << "handle_event_setparam max_reward_perday=" << m_param.max_reward_perday;
    m_store.den_param_put(transaction_a, m_param.max_reward_perday);
}

void mcp::den::handle_event_addminer(mcp::db::db_transaction & transaction_a, const log_entry &log_a, const uint64_t &time)
{
    dev::Address miner;
    den_unit u = den_unit(time);
    m_abi.UnpackEvent("AddMiner", log_a.data, miner);
    m_store.den_rewards_put(transaction_a, miner, u);
    m_den_witelist.emplace(miner);
    LOG(m_log.info) << "handle_event_addminer miner=" << miner.hexPrefixed();
}

void mcp::den::handle_event_deleteminer(mcp::db::db_transaction & transaction_a, const log_entry &log_a)
{

}

void mcp::den::handle_event_stake(mcp::db::db_transaction & transaction_a, const log_entry &log_a)
{

}

void mcp::den::handle_event_unstake(mcp::db::db_transaction & transaction_a, const log_entry &log_a)
{

}

mcp::den_event_type mcp::den::get_event_type(const std::string& eventName)
{
    den_event_type type;
    if(eventName == "SetParam"){
        type = EVENT_SET_PARAM;
    }
    else if(eventName == "AddMiner"){
        type = EVENT_ADD_MINER;
    }
    else if(eventName == "DeleteMiner"){
        type = EVENT_DELETE_MINER;
    }
    else if(eventName == "Stake"){
        type = EVENT_STAKE;
    }
    else if(eventName == "Unstake"){
        type = EVENT_UNSTAKE;
    }
    else{
        type = EVENT_UNKNOWN;
    }
    return type;
}

void mcp::den::handle_den_mining_event(mcp::db::db_transaction & transaction_a, const log_entries &log_a, const uint64_t &time)
{
    LOG(m_log.info) << "handle_den_mining_event in size=" << log_a.size();
    for(auto log : log_a){
        den_event_type type = get_event_type(m_abi.GetEventName(log.topics[0]));
        LOG(m_log.info) << "handle_den_mining_event type=" << (uint16_t)type;
        switch (type)
        {
        case EVENT_SET_PARAM:
            handle_event_setparam(transaction_a, log);
            break;
        case EVENT_ADD_MINER:
            handle_event_addminer(transaction_a, log, time);
            break;
        case EVENT_DELETE_MINER:
            handle_event_deleteminer(transaction_a, log);
            break;
        case EVENT_STAKE:
            handle_event_stake(transaction_a, log);
            break;
        case EVENT_UNSTAKE:
            handle_event_unstake(transaction_a, log);
            break;
        default:
            break;
        }
    }
}

// time in the block which include ping approve.
void mcp::den::handle_den_mining_ping(mcp::db::db_transaction & transaction_a, const dev::Address &addr, const uint64_t &time, bool ping, std::map<uint64_t, std::map<uint8_t, den_ping>>& pings)
{
    LOG(m_log.info) << "handle_den_mining_ping in " << addr.hexPrefixed() << " time:" << time;
    if(m_den_units.count(addr) == 0) return;
    auto &u = m_den_units[addr];
    uint64_t day = time / den_reward_period_day;
    uint32_t hour = time / den_reward_period % 24;
    LOG(m_log.info) << "[handle_den_mining_ping] day=" << day << " hour=" << hour << " ping=" << ping;

    for(uint64_t h=u.last_handle_ping_time/den_reward_period+1; h<time/den_reward_period; h++){
        mcp::block_hash hash;
        LOG(m_log.info) << "[handle_den_mining_ping] h="<<h<<" last_handle_ping_time="<<u.last_handle_ping_time<<" time="<<time;
        bool ret = m_store.den_period_mc_get(transaction_a, h, hash);
        if(ret){
            continue;
        }
        if(need_ping(addr, hash)) //need ping
        {
            pings[h / 24][h % 24] = {time, false};
            u.no_ping_times = 0;
            LOG(m_log.info) << "[handle_den_mining_ping] false1 day=" << h/24 << " hour=" << h%24;
        }
        else{
            u.no_ping_times++;
            if(u.no_ping_times >= 100){
                u.no_ping_times = 0;
                pings[h / 24][h % 24] = {time, false};
                LOG(m_log.info) << "[handle_den_mining_ping] false2 day=" << h/24 << " hour=" << h%24;
            }
        }
    }

    if(ping) {
        pings[day][hour] = {time, true};
        u.no_ping_times = 0;
    }

    u.last_handle_ping_time = time;
    LOG(m_log.info) << "handle_den_mining_ping out ";
}

bool mcp::den::calculate_rewards(const dev::Address &addr, const uint64_t time, dev::u256 &give_rewards, dev::u256 &frozen_rewards)
{
    LOG(m_log.info) << "[calculate_rewards] in addr=" << addr.hexPrefixed() << " time=" << time << " day=" << time / den_reward_period_day;
    mcp::db::db_transaction transaction(m_store.create_transaction());
    if(m_den_units.count(addr)){

        auto &u = m_den_units[addr];
        uint64_t cur_day = time / den_reward_period_day;
        if(cur_day <= u.last_calc_day){ //The call interval needs more than one day.
            LOG(m_log.info) << "[calculate_rewards] call interval less than one day.";
            return false;
        }

        //handle pings
        dev::h256s hashs;
        bool handle_ping_over = false;
        std::map<uint64_t, std::map<uint8_t, den_ping>> pings;
        //for X/24*24, need get all days ping, and den_rewards_put save the status in the end of last day.
        m_store.den_ping_get(transaction, addr, u.last_calc_day*24, hashs);
        for(auto h : hashs){
            std::shared_ptr<mcp::approve> a = m_store.approve_get(transaction, h);
            std::shared_ptr<mcp::block> b = m_store.block_get(transaction, a->hash());
            LOG(m_log.info) << "[calculate_rewards] ping mci:" << a->mci() << " hashs:" << a->hash().hexPrefixed();
            if(b->exec_timestamp()/den_reward_period_day < cur_day){
                handle_den_mining_ping(transaction, addr, b->exec_timestamp(), true, pings);
            }
            else{
                handle_den_mining_ping(transaction, addr, cur_day*den_reward_period_day , false, pings);
                handle_ping_over = true;
                break;
            }
        }
        if(!handle_ping_over){
            handle_den_mining_ping(transaction, addr, cur_day*den_reward_period_day, false, pings);
        }

        dev::u256 full_reward = m_param.max_reward_perday * u.stake_factor / 10000;
        bool & last_receive = u.last_receive;

        auto handle_no_ping_days = [&last_receive, &u, full_reward, this](uint64_t preday, uint64_t nextday){
            LOG(m_log.info) << "[HandleNoPingDays] Handle no ping days. preday=" << preday << " nextday=" << nextday;
            dev::u256 reward=0;
            for(int day=preday+1; day<nextday; day++){
                if(last_receive){
                    u.ping_lose_time = 0;
                    if(u.online_score < 10000){
                        uint32_t score = 0;
                        for(int i=1; i<=24; i++){
                            score += std::min((uint32_t)10000, u.online_score + i*10000/168);
                        }
                        reward = full_reward * score / 10000;
                        u.online_score = std::min((uint32_t)10000, u.online_score + 24*10000/168);
                    }
                    else{
                        reward = full_reward*24;
                    }
                }
                else{
                    reward = 0;
                    u.ping_lose_time += 24;
                    if(u.online_score > 10000*24/72){
                        u.online_score -= 10000*24/72;
                    }
                    else u.online_score = 0; 
                }
                u.frozen[day].frozen_reward = reward * 75 / 100;
                u.frozen[day].release_a_day = reward / 270;
                u.rewards += reward - u.frozen[day].frozen_reward;
                LOG(m_log.info) << "[HandleNoPingDays] day=" << day << " frozen_reward=" << u.frozen[day].frozen_reward.str() << " release_a_day=" << u.frozen[day].release_a_day.str();
            }
        };
        if(u.last_calc_day < pings.begin()->first){
            LOG(m_log.info) << "[calculate_rewards] last_calc_day < first";
            handle_no_ping_days(u.last_calc_day-1, pings.begin()->first);
        }
        for(std::map<uint64_t, std::map<uint8_t, den_ping>>::iterator it = pings.begin(); it != pings.end() && it->first < cur_day;)
        {
            uint8_t hour_pre = 0;
            uint8_t hour_next;
            std::map<uint8_t, den_ping>::iterator it2 = it->second.begin();
            if(it2->first == 0){
                last_receive = it2->second.receive;
                it2++;
                if(it2 == it->second.end()){
                    hour_next = 24;
                }
                else{
                    hour_next = it2->first;
                }
            }
            else{
                hour_next = it2->first;
            }
            LOG(m_log.info) << "[calculateOneday] day=" << it->first << " hour_next=" << (uint32_t)hour_next << " last_receive=" << last_receive;
            dev::u256 reward = 0;
            for(;;){
                if(last_receive){
                    u.ping_lose_time = 0;
                    if(u.online_score < 10000){
                        uint32_t score = 0;
                        for(int i=0; i<hour_next-hour_pre; i++){
                            score += std::min((uint32_t)10000, u.online_score + (i+1)*10000/168);
                        }
                        reward += full_reward * score / 10000;
                        u.online_score = std::min((uint32_t)10000, u.online_score + (hour_next-hour_pre)*10000/168);
                    }
                    else{
                        reward += full_reward*(hour_next-hour_pre);
                    }
                    LOG(m_log.info) << "[calculateOneday] reward=" << reward.str() << " online_score=" << u.online_score;
                }
                else{
                    u.ping_lose_time += 1;
                    if(u.ping_lose_time >= 2){
                        u.online_score = 0;
                    }
                    if(u.online_score > 10000/72){
                        u.online_score -= (hour_next-hour_pre)*10000/72;
                    }
                    else u.online_score = 0;
                    LOG(m_log.info) << "[calculateOneday] false and online_score=" << u.online_score << " ping_lose_time=" << u.ping_lose_time;
                }

                if(hour_next == 24){
                    LOG(m_log.info) << "[calculateOneday] all reward in day " << it->first << " v=" << reward.str() << " online_score=" << u.online_score;
                    break;
                }

                last_receive = it2->second.receive;
                it2++;
                hour_pre = hour_next;
                if(it2 != it->second.end()){
                    hour_next = it2->first;
                }
                else{
                    hour_next = 24;
                }
            }
            
            u.frozen[it->first].frozen_reward = reward * 75 / 100;
            u.frozen[it->first].release_a_day = reward / 270;
            LOG(m_log.info) << "[calculateOneday] frozen day=" << it->first << " frozen_reward=" << u.frozen[it->first].frozen_reward.str() << " release_a_day=" << u.frozen[it->first].release_a_day.str();
            u.rewards += reward - u.frozen[it->first].frozen_reward;
            uint64_t preday = it->first;
            uint64_t nextday;
            pings.erase(it++);

            if(it == pings.end()){
                nextday = cur_day;
            }
            else{
                nextday = it->first;
            }
            //Handle no ping days
            if(nextday - preday > 1){
                handle_no_ping_days(preday, nextday);
            }
        }

        //handle frozen rewards
        frozen_rewards = 0;
        LOG(m_log.info) << "[calculate_rewards] handle frozen rewards";
        for(std::map<uint64_t, den_reward_a_day>::iterator it = u.frozen.begin(); it != u.frozen.end() && it->first < cur_day;){
            LOG(m_log.info) << "[calculate_rewards] cur_day=" <<cur_day<<" it->first="<<it->first;
            if(cur_day - it->first >= 270){
                u.rewards += it->second.frozen_reward;
                u.frozen.erase(it++);
                continue;
            }
            auto give = it->second.release_a_day * (cur_day - std::max(u.last_calc_day, it->first));
            it->second.frozen_reward -= give;
            u.rewards += give;
            frozen_rewards += it->second.frozen_reward;
            LOG(m_log.info) << "[calculate_rewards] day=" << it->first << " frozen release=" << give.str();
            it++;
        }

        give_rewards = u.rewards;
        u.last_calc_day = time/mcp::den_reward_period_day;

        m_store.den_rewards_put(transaction, addr, u);
        LOG(m_log.info) << "[calculate_rewards] over give_rewards=" << give_rewards << " frozen_rewards=" << frozen_rewards;
        return true;
    }
    else{
        return false;
    }
}

uint64_t mcp::den::last_handle_ping_time(const dev::Address &addr)
{
    auto it = m_den_units.find(addr);
    assert_x(it!=m_den_units.end());

    return it->second.last_handle_ping_time;
}

bool mcp::den::need_ping(const dev::Address &addr, const block_hash &h)
{
    uint16_t ah = addr.data()[0];
    uint16_t hh = h.data()[0];
    LOG(g_log.info) << "[need_ping] addr=" << addr.hexPrefixed() << " h=" << h.hexPrefixed();
    LOG(g_log.info) << "[need_ping] random v=" << (((ah << 8) + addr.data()[1]) ^ ((hh << 8) + h.data()[1]));
	
    //For test
    //return (((ah << 8) + addr.data()[1]) ^ ((hh << 8) + h.data()[1])) < 65536/25;
    return true;
}

void mcp::den_unit::rewards_get(dev::RLP const & rlp)
{
    if (!rlp.isList())
        BOOST_THROW_EXCEPTION(InvalidTransactionFormat() << errinfo_comment("den unit RLP must be a list"));
    size_t count = rlp.itemCount();
    assert(count >= den_except_frozen_len);
    rewards = rlp[0].toInt<u256>();
    last_calc_day = rlp[1].toInt<uint64_t>();
    last_handle_ping_time = last_calc_day*mcp::den_reward_period_day;
    last_receive = rlp[2].toInt<bool>();
    no_ping_times = rlp[3].toInt<uint32_t>();
    ping_lose_time = rlp[4].toInt<uint32_t>();
    online_score = rlp[5].toInt<uint32_t>();
    LOG(m_log.info) << "[rewards_get] rewards=" << rewards.str() << " last_calc_day=" <<last_calc_day << " last_receive=" << last_receive;
    LOG(m_log.info) << "[rewards_get] no_ping_times=" << no_ping_times << " ping_lose_time=" << ping_lose_time << " online_score=" <<online_score;
    for(int n=0; n<count-den_except_frozen_len; n=n+2){
        frozen.emplace(last_calc_day-n-1, mcp::den_reward_a_day{rlp[n+den_except_frozen_len].toInt<u256>(), rlp[n+1+den_except_frozen_len].toInt<u256>()});
        LOG(m_log.info) << "[rewards_get] frozen day" << last_calc_day-n-1 << " frozen_reward=" << frozen[last_calc_day-n-1].frozen_reward << " release_a_day=" << frozen[last_calc_day-n-1].release_a_day;
    }
}

void mcp::den_unit::rewards_streamRLP(RLPStream& s)
{
    s.appendList(den_except_frozen_len + frozen.size()*2);
    LOG(m_log.info) << "[rewards_streamRLP] rewards=" << rewards.str() << " last_calc_day=" <<last_calc_day << " last_receive=" << last_receive;
    LOG(m_log.info) << "[rewards_streamRLP] no_ping_times=" << no_ping_times << " ping_lose_time=" << ping_lose_time  << " online_score=" <<online_score;
    s << rewards << last_calc_day << last_receive << no_ping_times << ping_lose_time << online_score;
    for(std::map<uint64_t, mcp::den_reward_a_day>::reverse_iterator it=frozen.rbegin(); it!=frozen.rend(); it++){
        s << it->second.release_a_day << it->second.frozen_reward;
        LOG(m_log.info) << "[rewards_streamRLP] frozen day=" << it->first << " frozen_reward=" << it->second.frozen_reward.str() << " release_a_day=" << it->second.release_a_day.str();
    }
}
