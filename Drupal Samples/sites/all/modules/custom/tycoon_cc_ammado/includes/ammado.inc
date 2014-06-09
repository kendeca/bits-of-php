<?php
/**
 * @file
 * Classes to support the ammado payment methods
 *
 */

//
// // Temp function to make constructor work
//   function variable_get($var,$value){
//       return $value;
//   }
//


/**
  * Payment method object
  *
  * @author Ken Y
  */
class TycoonCCAmmado
{

  /**
   * The transaction that triggered this ammado interaction
   *
   */

   protected $txid;

  /**
   * The flavor of ammado transaction we want:  widget or api
   *
   */

   protected $flavor;

   /**
    * The Ammado api key
    *
    */

   protected $api_key;

   /**
    * The Ammado api secret
    *
    */

   protected $api_secret;


  /**
   * Primary key: the internal id for this ammado transaction
   */

    protected $amid ; // serial


  /**
   * Ammado's order ID for this transaction
   */

    protected $order_id ; // varchar



 /**
   * Ammado's order salt for this transaction
   */

    protected $order_salt ; // varchar



  /**
   * Ammado\'s order status for this transaction
   */

    protected $order_status = " " ; // varchar



  /**
   * ISO 8601 date: YYYY-MM-DD
   */

    protected $order_date = "0000-00-00"; // char



  /**
   * 3 uppercase ISO currency code for this transaction
   */

    protected $currency_code ; // char



  /**
   * Donation amount for this transaction
   */

    protected $donation_amount ; // numeric


  /**
   * Unix timestamp when this entry was created
   */

    protected $timestamp ; // int



  /**
   * Serialized array of beneficiary ids
   */

    protected $beneficiary_array ; // text



  /**
   * Index to ammado API endpoint for this transaction
   */

    protected $aeid ; // int


  /**
   * API endpoint for this account
   */

    protected $endpoint ; // varchar



  /**
   * Widget code for this endpoint
   */

    protected $widget_code ; // text



  /**
   * Causecast billing id for this endpoint
   */

    protected $billing_id ; // varchar



  /**
   * API version for this endpoint
   */

    protected $api_version = 'v1'; // varchar



  /**
   * Database record for this object
   */

    protected $db_record = array(); // array



  /**
   * Database record for this object's endpoint data
   */

    protected $endpoints_db_record = array(); // array



  /**
   * Constructor for this class
   */
  function __construct($txid = 0,$flavor = 'widget') {
      //@todo Set up variables.  Make sure they are urlencoded.
      $this->api_key = (variable_get('cc_am_api_key','639A53D1-EAE0-4BC1-029C'));
      $this->api_secret = (variable_get('cc_am_api_secret','pDoEOM123459IEC'));
      $this->widget_code = (variable_get('cc_am_widget_code'
                                      ,'<div id="ammadoGivingWidget"></div>'
                                      .'<script type="text/javascript">'
                                      ."var s = document.createElement('script'); s.type='text/javascript'; "
                                      ."s.async=true;"
                                      ."s.src='https://www.ammadodemo.com/company/636/givingwidget/embed.js?renderTo=ammadoGivingWidget';"
                                      ."var f = document.getElementsByTagName('script')[0]; f.parentNode.insertBefore(s, f);</script>"));
      $this->endpoint = (variable_get('cc_am_endpoint','https://api.ammadodemo.com/v1/'));
      $this->xid = $txid ; // The transaction that triggered this ammado interaction
      $this->flavor = $flavor ; // The flavor of ammado transaction we want:  widget or api
      $this->currency_code  = 'USD' ; // 3 uppercase ISO currency code for this transaction // char
      $this->donation_amount  = 0.00 ; // Donation amount for this transaction // numeric
      $this->timestamp  =  0; // Unix timestamp when this entry was created // int
      $this->beneficiary_array  = array() ; // Serialized array of beneficiary ids // text
      $this->aeid  = 0 ; // Index to ammado API endpoint for this transaction // int
      $this->billing_id  = 'default' ; // Default billing id// varchar

      if ($txid > 0) $this->load_by_txid($txid);
    }



  /**
   * Load this widget from the database using the specified txid.
   * If the record does not exist, then create it!
   *
   * @param txid
   *   The Tycoon transaction id for this record.
   * @return
   *   This transaction if load is successful, false otherwise.
   */

   function load_by_txid($txid){

   // @todo Figure out why the regular dynamic database API did not work here!

   $output=true;

    if (isset($_REQUEST['ken_debug'])){
      watchdog('ken_debug',"In load_by_txid with txid =".$txid);
     }


    // Load the transaction
   $query = "select * from {tycoon_cc_ammado_orders} where "
              ."txid = ".$txid." "
              ."order by timestamp DESC LIMIT 1";

    $res = db_query($query);
           $this->db_record = $res->fetchAssoc();

        if (isset($_REQUEST['ken_debug'])){
        watchdog('ken_debug',
            "\n\n** in ammado load_by_txid \n\n this->db_record = \n".var_export($this->db_record,true));
        }

      if($this->db_record){
        // We have a record, set up the local variables
        $this->ammado_init_from_db_record();
      }
      else {
        // No record, create one.
        $this->db_record = Array();
        $this->db_record['txid'] = $txid;
        $this->db_record['timestamp'] = time();
        $this->db_record['aeid'] = $this->aeid;  // probably the default endpoint
        $this->db_record['amid'] = db_insert('tycoon_cc_ammado_orders')
          ->fields($this->db_record)
          ->execute();
        $this->txid = $txid;
        $this->amid = $this->db_record['amid'];
        $this->timestamp = $this->db_record['timestamp'];

      }

        // Now get the endpoint information

        // Load the endpoint
        if (($this->db_record['aeid'])>=0) {
          $query = "select * from {tycoon_cc_ammado_endpoints} where "
                ."aeid = ".($this->aeid)." ";
          $res = db_query($query);
          $this->endpoints_db_record = $res->fetchAssoc();
         if (isset($_REQUEST['ken_debug'])){
          watchdog('ken_debug',
              "\n\n** in ammado load_by_txid \n\n this->endpoints_db_record = \n"
                        .var_export($this->endpoints_db_record,true));
                      }
          $this->ammado_init_from_endpoints_db_record();
        }

      return $this;

   }

  /**
   * Save this object's data into the database
   */
  function save() {

    if (isset($_REQUEST['ken_debug'])){
      $err_msg = "\n\nEnter save in ammado.inc\n\n"
      ."\n\n\ammado object = ".var_export($this, 1)
      ;
      watchdog('ken_debug',$err_msg);

    }


    // Update the record with lastest values
    $this->ammado_update_db_record();
    $fields = $this->db_record;
    $amid = $fields['amid'];
    unset($fields['amid']);


      if (isset($_REQUEST['ken_debug'])){
        $err_msg = "\n\nabout to save in save() in ammado.inc\n\n"
        ."\n\n\ammado object = ".var_export($this, 1)
        ."\n\n\fields = ".var_export($fields, 1)
        ;
        watchdog('ken_debug',$err_msg);

      }


    $rows = db_update('tycoon_cc_ammado_orders')
      ->fields($this->db_record)
      ->condition('amid',$amid)
      ->execute();



      if (isset($_REQUEST['ken_debug'])){
        $err_msg = "\n\nSaved in save() in ammado.inc\n\n"
        ."\n\nrows = ".var_export($rows, 1)
        ;
        watchdog('ken_debug',$err_msg);

      }


    return $rows;
  }




  /**
   * Initialize the protected attributes from the $db_record array.
   */
  function ammado_init_from_db_record() {

    if (isset($_REQUEST['ken_debug'])){ watchdog('ken_debug',"In ammado_init_from_db_record with db_record = ".var_export($this->db_record,true)); }
     $this->amid  = $this->db_record['amid'] ; // Primary key: the internal id for this ammado transaction // serial
     $this->txid  = $this->db_record['txid'] ; // Causecast transaction id for this Ammado order // int
     $this->flavor = $this->db_record['flavor'] ; // The flavor of ammado transaction we want:  widget or api
     $this->order_id  = $this->db_record['order_id'] ; // Ammado's order ID for this transaction // varchar
     $this->order_status  = $this->db_record['order_status'] ; // Ammado\'s order status for this transaction // varchar
     $this->order_date  = $this->db_record['order_date'] ; // ISO 8601 date: YYYY-MM-DD // char
     $this->currency_code  = $this->db_record['currency_code'] ; // 3 uppercase ISO currency code for this transaction // char
     $this->donation_amount  = $this->db_record['donation_amount'] ; // Donation amount for this transaction // numeric
     $this->timestamp  = $this->db_record['timestamp'] ; // Unix timestamp when this entry was created // int
     $this->beneficiary_array  = unserialize($this->db_record['beneficiary_array']) ; // Serialized array of beneficiary ids // text
     $this->aeid  = $this->db_record['aeid'] ; // Index to ammado API endpoint for this transaction // int

    return;
  }


  /**
   * Initialize the protected attributes from the $endpoints_db_record    array.
   */
  function ammado_init_from_endpoints_db_record   () {

    if (isset($_REQUEST['ken_debug'])){ watchdog('ken_debug',"In ammado_init_from_endpoints_db_record    with endpoints_db_record    = ".var_export($this->endpoints_db_record   ,true)); }
     $this->aeid  = $this->endpoints_db_record   ['aeid'] ; // Index to ammado API endpoint for this transaction // int
     $this->endpoint  = $this->endpoints_db_record   ['endpoint'] ; // API endpoint for this account // varchar
     $this->widget_code  = $this->endpoints_db_record   ['widget_code'] ; // Widget code for this endpoint // text
     $this->billing_id  = $this->endpoints_db_record   ['billing_id'] ; // Causecast billing id for this endpoint // varchar

    return;
  }


 /**
   * Update the $this->db_record array after data changes
   */
  function ammado_update_db_record() {
      $this->db_record['amid'] = $this->amid  ; // Primary key: the internal id for this ammado transaction // serial
      $this->db_record['txid'] = $this->txid ; // The transaction that triggered this ammado interaction
      $this->db_record['flavor'] = $this->flavor ; // The flavor of ammado transaction we want:  widget or api
      $this->db_record['order_id'] = $this->order_id  ; // Ammado's order ID for this transaction // varchar
      $this->db_record['order_status'] = $this->order_status  ; // Ammado\'s order status for this transaction // varchar
      $this->db_record['order_date'] = $this->order_date  ; // ISO 8601 date: YYYY-MM-DD // char
      $this->db_record['currency_code'] = $this->currency_code  ; // 3 uppercase ISO currency code for this transaction // char
      $this->db_record['donation_amount'] = $this->donation_amount  ; // Donation amount for this transaction // numeric
      $this->db_record['timestamp'] = $this->timestamp  ; // Unix timestamp when this entry was created // int
      $this->db_record['beneficiary_array'] = serialize($this->beneficiary_array ) ; // Serialized array of beneficiary ids // text
      $this->db_record['aeid'] = $this->aeid  ; // Index to ammado API endpoint for this transaction // int

    return;
  }

 /**
   * Update the $this->endpoints_db_record    array after data changes
   */
  function ammado_update_endpoints_db_record   () {
      $this->endpoints_db_record   ['aeid'] = $this->aeid  ; // Primary key: the endpoint number // serial,
      $this->endpoints_db_record   ['endpoint'] = $this->endpoint  ; // API endpoint for this account // varchar
      $this->endpoints_db_record   ['widget_code'] = $this->widget_code  ; // Widget code for this endpoint // text
      $this->endpoints_db_record   ['billing_id'] = $this->billing_id  ; // Causecast billing id for this endpoint // varchar

    return;
  }


/**
 * Update the Ammado order status given the values in an array.
 */
function ammado_update_order_status($status) {
  if($this->order_id == $status['orderId']){
    $this->order_status = $status['orderStatus'];
    $this->order_date = $status['orderDate'];
    $this->donation_amount = isset($status['donationAmount'])?$status['donationAmount']:0.00;
    $this->currency_code = isset($status['currencyCode'])?$status['currencyCode']:'USD';
    $this->beneficiary_array = isset($status['beneficiaryId'])?
        (is_array($status['beneficiaryId'])?$status['beneficiaryId']:array($status['beneficiaryId']))
        :array();

    $this->ammado_update_db_record();
    $this->save();
  } else return false;

  return $this;
}

    /**
   * Getter for $txid
   * The transaction that triggered this ammado interaction
   */

  function get_txid(){
    return $this->txid ;
  }

  /**
   * Setter for $txid
   * The transaction that triggered this ammado interaction
   */

  function set_txid($txid){
    $this->txid = $txid ;
    return $this->txid ;
  }




  /**
   * Getter for $flavor
   * The flavor of ammado transaction we want:  widget or api
   */

  function get_flavor(){
    return $this->flavor ;
  }

  /**
   * Setter for $flavor
   * The flavor of ammado transaction we want:  widget or api
   */

  function set_flavor($flavor){
    $this->flavor = $flavor ;
    return $this->flavor ;
  }


  /**
   * Getter for $api_key
   * The Ammado api key
   */

  function get_api_key(){
    return $this->api_key ;
  }

  /**
   * Setter for $api_key
   * The Ammado api key
   */

  function set_api_key($api_key){
    $this->api_key = $api_key ;
    return $this->api_key ;
  }




  /**
   * Getter for $api_secret
   * The Ammado api secret
   */

  function get_api_secret(){
    return $this->api_secret ;
  }

  /**
   * Setter for $api_secret
   * The Ammado api secret
   */

  function set_api_secret($api_secret){
    $this->api_secret = $api_secret ;
    return $this->api_secret ;
  }

    /**
   * Getter for $amid
   * Primary key: the internal id for this ammado transaction
   */

  function get_amid (){
    return $this->amid  ;
  }

  /**
   * Setter for $amid
   * Primary key: the internal id for this ammado transaction
   */

  function set_amid ($amid ){
    $this->amid  = $amid  ;
    return $this->amid  ;
  }


  /**
   * Getter for $order_id
   * Ammado's order ID for this transaction
   */

  function get_order_id (){
    return $this->order_id  ;
  }

  /**
   * Setter for $order_id
   * Ammado's order ID for this transaction
   */

  function set_order_id ($order_id ){
    $this->order_id  = $order_id  ;
    return $this->order_id  ;
  }


 /**
   * Setter for $order_salt
   * Ammado's order salt for this widget
   */

  function set_order_salt ($order_salt ){
    $this->order_salt  = $order_salt  ;
    return $this->order_salt  ;
  }


 /**
   * Getter for $order_salt
   * Ammado's order salt for this widget
   */

  function get_order_salt (){
    return $this->order_salt  ;
  }


  /**
   * Getter for $order_status
   * Ammado\'s order status for this transaction
   */

  function get_order_status (){
    return $this->order_status  ;
  }

  /**
   * Setter for $order_status
   * Ammado\'s order status for this transaction
   */

  function set_order_status ($order_status ){
    $this->order_status  = $order_status  ;
    return $this->order_status  ;
  }


  /**
   * Getter for $order_date
   * ISO 8601 date: YYYY-MM-DD
   */

  function get_order_date (){
    return $this->order_date  ;
  }

  /**
   * Setter for $order_date
   * ISO 8601 date: YYYY-MM-DD
   */

  function set_order_date ($order_date ){
    $this->order_date  = $order_date  ;
    return $this->order_date  ;
  }


  /**
   * Getter for $currency_code
   * 3 uppercase ISO currency code for this transaction
   */

  function get_currency_code (){
    return $this->currency_code  ;
  }

  /**
   * Setter for $currency_code
   * 3 uppercase ISO currency code for this transaction
   */

  function set_currency_code ($currency_code ){
    $this->currency_code  = $currency_code  ;
    return $this->currency_code  ;
  }


  /**
   * Getter for $donation_amount
   * Donation amount for this transaction
   */

  function get_donation_amount (){
    return $this->donation_amount  ;
  }

  /**
   * Setter for $donation_amount
   * Donation amount for this transaction
   */

  function set_donation_amount ($donation_amount ){
    $this->donation_amount  = $donation_amount  ;
    return $this->donation_amount  ;
  }

    /**
   * Getter for $timestamp
   * Unix timestamp when this entry was created
   */

  function get_timestamp (){
    return $this->timestamp  ;
  }

  /**
   * Setter for $timestamp
   * Unix timestamp when this entry was created
   */

  function set_timestamp ($timestamp ){
    $this->timestamp  = $timestamp  ;
    return $this->timestamp  ;
  }


  /**
   * Getter for $beneficiary_array
   * Serialized array of beneficiary ids
   */

  function get_beneficiary_array (){
    return $this->beneficiary_array  ;
  }

  /**
   * Setter for $beneficiary_array
   * Serialized array of beneficiary ids
   */

  function set_beneficiary_array ($beneficiary_array ){
    $this->beneficiary_array  = $beneficiary_array  ;
    return $this->beneficiary_array  ;
  }


  /**
   * Getter for $aeid
   * Index to ammado API endpoint for this transaction
   */

  function get_aeid (){
    return $this->aeid  ;
  }

    /**
   * Getter for $endpoint
   * API endpoint for this account
   */

  function get_endpoint (){
    return $this->endpoint  ;
  }

  /**
   * Setter for $endpoint
   * API endpoint for this account
   */

  function set_endpoint ($endpoint ){
    $this->endpoint  = $endpoint  ;
    return $this->endpoint  ;
  }


  /**
   * Getter for $widget_code
   * Widget code for this endpoint
   */

  function get_widget_code (){
    return $this->widget_code  ;
  }

  /**
   * Setter for $widget_code
   * Widget code for this endpoint
   */

  function set_widget_code ($widget_code ){
    $this->widget_code  = $widget_code  ;
    return $this->widget_code  ;
  }


  /**
   * Getter for $billing_id
   * Causecast billing id for this endpoint
   */

  function get_billing_id (){
    return $this->billing_id  ;
  }

  /**
   * Setter for $billing_id
   * Causecast billing id for this endpoint
   */

  function set_billing_id ($billing_id ){
    $this->billing_id  = $billing_id  ;
    return $this->billing_id  ;
  }


  /**
   * Getter for $api_version
   * API version for this endpoint
   */

  function get_api_version(){
    return $this->api_version ;
  }

  /**
   * Setter for $api_version
   * API version for this endpoint
   */

  function set_api_version($api_version){
    $this->api_version = $api_version;
    return $this->api_version;
  }



  /**
   * Getter for $db_record
   * API version for this endpoint
   */

  function get_db_record(){
    return $this->db_record ;
  }

  /**
   * Setter for $db_record
   * API version for this endpoint
   */

  function set_db_record($db_record){
    $this->db_record = $db_record;
    return $this->db_record;
  }

  /**
   * Getter for $endpoints_db_record
   * API version for this endpoint
   */

  function get_endpoints_db_record(){
    return $this->endpoints_db_record ;
  }

  /**
   * Setter for $endpoints_db_record
   * API version for this endpoint
   */

  function set_endpoints_db_record($endpoints_db_record){
    $this->endpoints_db_record = $endpoints_db_record;
    return $this->endpoints_db_record;
  }




}
