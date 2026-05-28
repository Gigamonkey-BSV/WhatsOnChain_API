#ifndef COSMOS_NETWORK_WHATSONCHAIN
#define COSMOS_NETWORK_WHATSONCHAIN

#include <net/JSON.hpp>
#include <net/HTTP_client.hpp>

#include <gigamonkey/secp256k1.hpp>
#include <gigamonkey/merkle/proof.hpp>
#include <gigamonkey/satoshi.hpp>
#include <gigamonkey/timechain.hpp>
#include <gigamonkey/address.hpp>

namespace WhatsOnChain {

    template <typename X> using awaitable = data::awaitable<X>;

    using data::synced;

    /*
     * any function f that returns an awaitable<X> is an async
     * function, which means that the function is scheduled to
     * run at some unspecified time.
     *
     * If you are within an async function, use
     *    co_await f (arguments_of_f...);
     * to wait for the async operation to complete and return
     * an X.
     *
     * If you are not within an async function, use
     *    synced (f, arguments_of_f...);
     * to schedule the async function and immediately wait to
     * get an X. Function f will run in the same thread.
     */

    // the WhatsOnChain API.
    struct API;

    // to start use
    //    API api ();          // free account
    //    API api ("API key");
    //    auto tx = api.transactions ().get_raw (txid);

    namespace Bitcoin = Gigamonkey::Bitcoin;
    using JSON = net::JSON;

    using uint32 = data::uint32;

    struct UTXO {

        Bitcoin::outpoint Outpoint;
        Bitcoin::satoshi Value;
        uint32 Height;

        UTXO ();
        UTXO (const JSON &);
        bool valid () const;

        bool operator == (const UTXO &u) const {
            return Outpoint == u.Outpoint && Value == u.Value && Height == u.Height;
        }

        explicit operator JSON () const;

        friend std::ostream inline &operator << (std::ostream &o, const UTXO &u) {
            return o << "UTXO {" << u.Outpoint << ", " << u.Value << ", " << u.Height << "}";
        }

    };

    // iterable functional list
    template <typename X> using list = data::list<X>;

    struct addresses {
        struct balance {
            Bitcoin::satoshi Confirmed;
            Bitcoin::satoshi Unconfirmed;
        };

        awaitable<balance> get_balance (const Bitcoin::address &);

        // txids of all transactions that spend to or redeem from a given address.
        awaitable<list<Bitcoin::TxID>> get_history (const Bitcoin::address &);

        awaitable<list<UTXO>> get_unspent (const Bitcoin::address &address);

        WhatsOnChain::API &API;
    };

    namespace Merkle = Gigamonkey::Merkle;
    using digest256 = Gigamonkey::digest256;

    struct merkle_proof {
        digest256 BlockHash;
        Merkle::proof Proof;
    };

    using bytes = data::bytes;
    template <typename X> using maybe = data::maybe<X>;

    struct transactions {

        awaitable<bool> broadcast (const bytes& tx);

        awaitable<bytes> get_raw (const Bitcoin::TxID &);

        awaitable<JSON> tx_data (const Bitcoin::TxID &);

        awaitable<maybe<merkle_proof>> get_merkle_proof (const Bitcoin::TxID &);

        WhatsOnChain::API &API;
    };

    // a uint unbounded in size.
    using N = data::N;

    struct header {
        N Height;
        Bitcoin::header Header;
        header ();
        header (const N &n, const Bitcoin::header &h);
        bool valid () const;
    };

    struct blocks {
        // by block hash, not by merkle root.
        awaitable<header> get_header_by_hash (const digest256 &);

        // by height
        awaitable<header> get_header_by_height (const N &);

        WhatsOnChain::API &API;
    };

    struct scripts {

        awaitable<list<UTXO>> get_unspent (const digest256 &script_hash);
        awaitable<list<Bitcoin::TxID>> get_history (const digest256 &script_hash);

        WhatsOnChain::API &API;

    };

    using uint64 = data::uint64;
    using int64 = data::int64;
    using int32 = data::int32;

    using uint16 = data::uint16;

    using byte = data::byte;

    using int32_little = data::int32_little;
    using uint32_little = data::uint32_little;

    using string = data::string;

    using digest512 = Gigamonkey::digest512;
    using digest160 = Gigamonkey::digest160;

    template <typename X> using ptr = data::ptr<X>;
    template <typename ... X> using either = data::either<X...>;

    struct API : net::HTTP::client {

        API (): net::HTTP::client {net::HTTP::get_SSL (),
            net::HTTP::REST {"api.whatsonchain.com", "/v1/bsv/main"},
            data::rate_limiter {3, data::millisecond {1000}}} {}

        WhatsOnChain::addresses addresses ();

        WhatsOnChain::transactions transactions ();

        WhatsOnChain::scripts scripts ();

        WhatsOnChain::blocks blocks ();

    };

    addresses inline API::addresses () {
        return WhatsOnChain::addresses {*this};
    }

    transactions inline API::transactions () {
        return WhatsOnChain::transactions {*this};
    }

    scripts inline API::scripts () {
        return WhatsOnChain::scripts {*this};
    }

    blocks inline API::blocks () {
        return WhatsOnChain::blocks {*this};
    }

    bool inline UTXO::valid () const {
        return Value != 0;
    }

    inline header::header () : Height {0}, Header {} {}

    inline header::header (const N &n, const Bitcoin::header &h): Height {n}, Header {h} {}

    bool inline header::valid () const {
        return Header.valid ();
    }

    inline UTXO::UTXO () : Outpoint {}, Value {}, Height {} {}
}

#endif

