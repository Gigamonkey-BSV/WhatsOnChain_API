#ifndef COSMOS_NETWORK_WHATSONCHAIN
#define COSMOS_NETWORK_WHATSONCHAIN

#include <data/net/JSON.hpp>
#include <data/net/HTTP_client.hpp>

#include <gigamonkey/secp256k1.hpp>
#include <gigamonkey/merkle/proof.hpp>
#include <gigamonkey/satoshi.hpp>
#include <gigamonkey/timechain.hpp>
#include <gigamonkey/address.hpp>

namespace WhatsOnChain {
    namespace net = data::net;

    namespace Bitcoin = Gigamonkey::Bitcoin;
    namespace Merkle = Gigamonkey::Merkle;
    namespace secp256k1 = Gigamonkey::secp256k1;

    using uint32 = data::uint32;
    using uint64 = data::uint64;
    using int64 = data::int64;
    using int32 = data::int32;

    using uint16 = data::uint16;

    using byte = data::byte;
    using bytes = data::bytes;

    using int32_little = data::int32_little;
    using uint32_little = data::uint32_little;

    using string = data::string;
    using N = data::N;
    using JSON = data::JSON;

    using digest512 = Gigamonkey::digest512;
    using digest256 = Gigamonkey::digest256;
    using digest160 = Gigamonkey::digest160;

    template <typename X> using ptr = data::ptr<X>;
    template <typename X> using awaitable = data::awaitable<X>;
    template <typename X> using maybe = data::maybe<X>;
    template <typename ... X> using either = data::either<X...>;

    template <typename X> using list = data::list<X>;

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

    struct API;

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

    struct merkle_proof {
        digest256 BlockHash;
        Merkle::proof Proof;
    };

    struct transactions {

        awaitable<bool> broadcast (const bytes& tx);

        awaitable<bytes> get_raw (const Bitcoin::TxID &);

        awaitable<JSON> tx_data (const Bitcoin::TxID &);

        awaitable<maybe<merkle_proof>> get_merkle_proof (const Bitcoin::TxID &);

        WhatsOnChain::API &API;
    };

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

    struct API : net::HTTP::client {

        API (ptr<net::HTTP::SSL> ssl) :
            net::HTTP::client {ssl,
                net::HTTP::REST {"api.whatsonchain.com", "/v1/bsv/main"},
                data::rate_limiter {3, data::millisecond {1000}}} {}

        API (): net::HTTP::client {
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

