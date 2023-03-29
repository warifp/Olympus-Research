#include "den.hpp"
#include <algorithm>
#include <map>
#include <mcp/core/block_store.hpp>
#include <mcp/core/approve.hpp>
#include "mcp/rpc/jsonHelper.hpp"
#include "mcp/node/message.hpp"

std::shared_ptr<mcp::den> mcp::g_den;

mcp::den::den(mcp::block_store& store_a) :
    m_store(store_a)
{
    den_unit u;
    u.stake_factor = 1;
    m_dens.emplace(jsToAddress("0x1144B522F45265C2DFDBAEE8E324719E63A1694C"), u);
    m_dens.emplace(jsToAddress("0xBAC0b1DD15093De9bBEb8f2E940eb0872A4E0bCD"), u);
    m_dens.emplace(jsToAddress("0x740233e47e13a30B650f3E5Bf59DCcc8Fd16B373"), u);
}

void mcp::den::init(mcp::db::db_transaction & transaction_a)
{
    LOG(m_log.info) << "[den::init] in";
    for(auto u : m_dens){
        if(!m_store.den_rewards_get(transaction_a, u.first, u.second)){
            LOG(m_log.info) << "[den::init] last_calc_time=" << u.second.last_calc_time;
            for(auto f : u.second.frozen){
                LOG(m_log.info) << "[den::init] day=" << f.first << " v=" << f.second;
            }
            LOG(m_log.info) << "[den::init] den_rewards get ok. " << u.first.hexPrefixed();
        }
        else{
            LOG(m_log.info) << "[den::init] den_rewards not get. " << u.first.hexPrefixed();
            u.second.last_calc_time = 1679994801;
            u.second.last_handle_ping_time = 1679994801;
        }
        
        dev::h256s hashs;
        LOG(m_log.info) << "[den::init] den_ping_get hour=" << u.second.last_calc_time/den_reward_period+1;
        m_store.den_ping_get(transaction_a, u.first, u.second.last_calc_time/den_reward_period+1, hashs);
        for(auto h : hashs){
            LOG(m_log.info) << "[den::init] h=" << h.hexPrefixed();
            std::shared_ptr<mcp::approve> a = m_store.approve_get(transaction_a, h);
            LOG(m_log.info) << "[den::init] ping mci:" << a->mci() << " hashs:" << a->hash().hexPrefixed();
            std::shared_ptr<mcp::block> b = m_store.block_get(transaction_a, a->hash());
            handle_den_mining_ping(transaction_a, u.first, b->exec_timestamp(), true);
        }
    }
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

void mcp::den::handle_den_mining_event(const log_entries &log_a)
{
    LOG(m_log.info) << "handle_den_mining_event in size=" << log_a.size();
    for(size_t i=0; i<log_a.size(); i++){
        LOG(m_log.info) << "i=" << i << " " << dev::toHex(log_a[i].data) << " address=" << log_a[i].address.hexPrefixed();
    }
}

// time in the block which include ping approve.
void mcp::den::handle_den_mining_ping(mcp::db::db_transaction & transaction_a, const dev::Address &addr, const uint64_t &time, bool ping)
{
    LOG(m_log.info) << "handle_den_mining_ping in " << addr.hexPrefixed() << " time:" << time;
    if(m_dens.count(addr) == 0) return;
    auto &u = m_dens[addr];
    auto &pings = m_dens[addr].pings;
    uint64_t day = time / den_reward_period / 24;
    uint64_t hour = time / den_reward_period % 24;
    LOG(m_log.info) << "[handle_den_mining_ping] day=" << day << " hour=" << hour << " ping=" << ping;

    if(ping) pings[day][hour] = {time, true};

    for(uint64_t h=u.last_handle_ping_time/den_reward_period+1; h<time/den_reward_period; h++){
        mcp::block_hash hash;
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
    u.last_handle_ping_time = time;
    LOG(m_log.info) << "handle_den_mining_ping out ";
}

bool mcp::den::calculate_rewards(const dev::Address &addr, const uint64_t time, dev::u256 &give_rewards, dev::u256 &frozen_rewards, bool provide)
{
    LOG(m_log.info) << "calculate_rewards in ";
    mcp::db::db_transaction transaction(m_store.create_transaction());
    if(m_dens.count(addr)){
        handle_den_mining_ping(transaction, addr, time, false);

        auto &u = m_dens[addr];
        uint64_t cur_day = time / den_reward_period / 24;
        uint64_t last_calc_day = u.last_calc_time/den_reward_period/24;
        if(cur_day <= last_calc_day){ //The call interval needs more than one day.
            return false;
        }

        static uint32_t ping_lose_time = 0;
        dev::u256 all_reward = 0;
        dev::u256 full_reward = m_param.max_reward_perday * u.stake_factor / 10000;
        bool & last_receive = u.last_receive;
        for(std::map<uint64_t, std::map<uint8_t, den_ping>>::iterator it = u.pings.begin(); it != u.pings.end() && it->first < cur_day;){
            if(it->second.empty()){  //There is no ping on this day.
                if(last_receive){
                    if(u.online_score < 10000){
                        uint32_t score = 0;
                        for(int i=1; i<=24; i++){
                            score += std::min((uint32_t)10000, u.online_score + i*10000/168);
                        }
                        all_reward += full_reward * score / 10000;
                        u.online_score = std::min((uint32_t)10000, u.online_score + 24*10000/168);
                    }
                    else{
                        all_reward += full_reward*24;
                    }
                }
                else{
                    if(u.online_score > 10000*24/72){
                        u.online_score -= 10000*24/72;
                    }
                    else u.online_score = 0; 
                }
            }
            else
            {
                uint8_t hour_last = 0;
                uint8_t now;
                std::map<uint8_t, den_ping>::iterator it2;
                for(it2=it->second.begin(), now=it2->first; now <= 23;){
                    if(last_receive){
                        if(u.online_score < 10000){
                            uint32_t score = 0;
                            for(int i=1; i<=now-hour_last; i++){
                                score += std::min((uint32_t)10000, u.online_score + i*10000/168);
                            }
                            all_reward += full_reward * score / 10000;
                            u.online_score = std::min((uint32_t)10000, u.online_score + (now-hour_last)*10000/168);
                        }
                        else{
                            all_reward += full_reward*(now-hour_last);
                        }
                    }
                    else{
                        if(u.online_score > 10000/72){
                            u.online_score -= 10000/72;
                        }
                        else u.online_score = 0;
                    }

                    last_receive = it2->second.receive;
                    if(last_receive){
                        ping_lose_time = 0;
                    }
                    else{
                        ping_lose_time += 1;
                        if(ping_lose_time >= 2){
                            u.online_score = 0;
                        }
                    }

                    it2++;
                    hour_last = now;
                    if(it2 != it->second.end()){
                        now = it2->first;
                    }
                    else{
                        now = 24;
                    }
                }
            }
            u.frozen[it->first] = all_reward * 75 / 100;
            LOG(m_log.info) << "[calculate_rewards] frozen day=" << it->first << " v=" << u.frozen[it->first].str();
            u.rewards += all_reward * 25 / 100;
            all_reward = 0;
            u.pings.erase(it++);
        }

        for(std::map<uint64_t, dev::u256>::iterator it = u.frozen.begin(); it != u.frozen.end() && it->first < cur_day;){
            if(cur_day - it->first >= 270){
                u.rewards += it->second;
                u.frozen.erase(it++);
                continue;
            }
            auto release_perday = it->second / (270 - (last_calc_day - it->first));
            auto give = release_perday * (cur_day-std::max(last_calc_day, it->first));
            it->second -= give;
            u.rewards += give;
            frozen_rewards += it->second;
            it++;
        }

        give_rewards = u.rewards;
        if(provide){
            u.rewards = 0;
        }
        u.last_calc_time = time;

        LOG(m_log.info) << "call den_rewards_put " << addr.hexPrefixed();
        m_store.den_rewards_put(transaction, addr, u);
        LOG(m_log.info) << "[calculate_rewards] give_rewards=" << give_rewards << " frozen_rewards=" << frozen_rewards;
        return true;
    }
    else{
        return false;
    }
}

uint64_t mcp::den::last_handle_ping_time(const dev::Address &addr)
{
    auto it = m_dens.find(addr);
    assert_x(it!=m_dens.end());

    return it->second.last_handle_ping_time;
}

bool mcp::den::need_ping(const dev::Address &addr, const block_hash &h)
{
    uint16_t ah = addr.data()[0];
    uint16_t hh = h.data()[0];
    //return (((ah << 8) + addr.data()[1]) ^ ((hh << 8) + h.data()[1])) < 65536/25;
    return true;
}

void mcp::den_unit::rewards_get(dev::RLP const & rlp)
{
    if (!rlp.isList())
        BOOST_THROW_EXCEPTION(InvalidTransactionFormat() << errinfo_comment("den unit RLP must be a list"));
    size_t count = rlp.itemCount();
    assert(count >= 5);
    rewards = rlp[0].toInt<u256>();
    last_calc_time = rlp[1].toInt<uint64_t>();
    last_receive = rlp[2].toInt<bool>();
    no_ping_times = rlp[3].toInt<uint32_t>();
    online_score = rlp[4].toInt<uint32_t>();
    LOG(m_log.info) << "[rewards_get] rewards=" << rewards.str() << " last_calc_time=" <<last_calc_time << " last_receive=" << last_receive;
    LOG(m_log.info) << "[rewards_get] no_ping_times=" << no_ping_times << " online_score=" <<online_score;
    uint64_t last_calc_day = last_calc_time/den_reward_period/24-1;
    for(int n=5; n<count; n++){
        frozen.emplace(last_calc_day-(n-5), rlp[n].toInt<u256>());
        LOG(m_log.info) << "[rewards_get] frozen day" << last_calc_day-(n-5) << " =" << rlp[n].toInt<u256>().str();
    }
}

void mcp::den_unit::rewards_streamRLP(RLPStream& s)
{
    s.appendList(5 + frozen.size());
    s << rewards << last_calc_time << last_receive << no_ping_times << online_score;
    for(std::map<uint64_t, dev::u256>::reverse_iterator it=frozen.rbegin(); it!=frozen.rend(); it++){
        s << it->second;
    }
}
