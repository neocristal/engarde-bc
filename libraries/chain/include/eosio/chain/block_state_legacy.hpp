#pragma once

#include <eosio/chain/block_header_state_legacy.hpp>
#include <eosio/chain/block.hpp>
#include <eosio/chain/transaction_metadata.hpp>
#include <eosio/chain/action_receipt.hpp>

namespace eosio { namespace chain {

   struct block_state_legacy : public block_header_state_legacy {
      block_state_legacy( const block_header_state_legacy& prev,
                          signed_block_ptr b,
                          const protocol_feature_set& pfs,
                          const std::function<void( block_timestamp_type,
                                                    const flat_set<digest_type>&,
                                                    const vector<digest_type>& )>& validator,
                          bool skip_validate_signee
                 );

      block_state_legacy( pending_block_header_state_legacy&& cur,
                          signed_block_ptr&& b, // unsigned block
                          deque<transaction_metadata_ptr>&& trx_metas,
                          const protocol_feature_set& pfs,
                          const std::function<void( block_timestamp_type,
                                                    const flat_set<digest_type>&,
                                                    const vector<digest_type>& )>& validator,
                          const signer_callback_type& signer
                );

      block_state_legacy() = default;


      signed_block_ptr                                    block;

   private: // internal use only, not thread safe
      friend struct fc::reflector<block_state_legacy>;
      friend bool block_state_is_valid( const block_state_legacy& ); // work-around for multi-index access
      friend struct controller_impl;
      friend class  fork_database;
      friend struct fork_database_impl;
      friend class  unapplied_transaction_queue;
      friend struct pending_state;

      bool is_valid()const { return validated; }
      bool is_pub_keys_recovered()const { return _pub_keys_recovered; }

      deque<transaction_metadata_ptr> extract_trxs_metas() {
         _pub_keys_recovered = false;
         auto result = std::move( _cached_trxs );
         _cached_trxs.clear();
         return result;
      }
      void set_trxs_metas( deque<transaction_metadata_ptr>&& trxs_metas, bool keys_recovered ) {
         _pub_keys_recovered = keys_recovered;
         _cached_trxs = std::move( trxs_metas );
      }
      const deque<transaction_metadata_ptr>& trxs_metas()const { return _cached_trxs; }

      bool                                                validated = false;

      bool                                                _pub_keys_recovered = false;
      /// this data is redundant with the data stored in block, but facilitates
      /// recapturing transactions when we pop a block
      deque<transaction_metadata_ptr>                    _cached_trxs;
   };

   using block_state_legacy_ptr = std::shared_ptr<block_state_legacy>;
   using branch_type = deque<block_state_legacy_ptr>;

} } /// namespace eosio::chain

FC_REFLECT_DERIVED( eosio::chain::block_state_legacy, (eosio::chain::block_header_state_legacy), (block)(validated) )
