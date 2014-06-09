<?php
/**
 * @file
 * Definition of TycoonCCAmmadoWidgetMethod
 */

/**
  * Class for Ammado Widget payments
  *
  * TycoonCCAmmadoWidgetMethod is the class for the ammado widget pay method.
  *
  */
class TycoonCCAmmadoWidgetMethod extends TycoonCCAmmadoMethod {

 /**
   * Constant to make the id of this class easy to get
   */
  const pmid = 'tycoon_cc_ammado_widget';

  /**
   * Constructor class.  Set the id.
   */
  function __construct() {
    parent::__construct();

    $this->id = self::pmid;
    $this->payment_method_id = self::pmid;

  }


  /**
   * Initializes the class
   */
  function init($options) {
    parent::init($options);

  }

  function validate(&$transaction, &$data) {
    $retval = parent::validate($transaction, $data);
    //@todo Validate functions specific to Widget payment
    // The validate process will require us to go to ammado, come back, and process the payment.

    return($retval);
  }
  /* end validate */

  function process(&$transaction, &$data) {

    if (isset($_REQUEST['ken_debug'])){
      $err_msg = "In process in ammado_widget.inc\n\n"
      ."\n\nTransaction = ".var_export($transaction, 1)
      ."\n\nData = ".var_export($data, 1)
      ."\n\nthis = ".var_export($this, 1);
      watchdog('ken_debug',$err_msg);

    }


    parent::process($transaction, $data);

    return $data;
  }

  function settle(&$transaction, &$data) {
    parent::settle($transaction, $data);
    //This will be called to settle things when the transaction settles.

    if (isset($_REQUEST['ken_debug'])){
      $err_msg = "In settle in ammado_widget.inc\n\n"
      ."\n\nTransaction = ".var_export($transaction, 1)
      ."\n\nData = ".var_export($data, 1)
      ."\n\nrecur = ".(isset($transaction->recur)?var_export($transaction->recur, 1):" RECUR not set")
      ."\n\nthis = ".var_export($this, 1);
      watchdog('ken_debug',$err_msg);

    }

    return $data;
  }

  function succeed(&$transaction, &$data) {


    if (isset($_REQUEST['ken_debug'])){
      $err_msg = "In succeed in ammado_widget.inc\n\n"
      ."\n\nTransaction = ".var_export($transaction, 1)
      ."\n\nData = ".var_export($data, 1)
      ."\n\nrecur = ".(isset($transaction->recur)?var_export($transaction->recur, 1):" RECUR not set")
      ."\n\nthis = ".var_export($this, 1);
      watchdog('ken_debug',$err_msg);

    }


    parent::succeed($transaction, $data);

    return $data;
  }

  function tokens($tokens, $options = array()) {
    $replacements = array();
    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'name':
          $replacements[$original] = t('Ammado', array(), $options);
          break;
      }
    }
    return $replacements;
  }


  //@todo modify this schema for Ammado widget payments
  function schema() {
    return array(
      'first_name' => array(
        'type' => 'varchar',
        'title' => t('First name'),
        'required' => FALSE,
        'maxlength' => 50,
      ),
      'last_name' => array(
        'type' => 'varchar',
        'title' => t('Last name'),
        'required' => FALSE,
        'maxlength' => 50,
      ),
      'mail' => array(
        'type' => 'varchar',
        'title' => t('Email'),
        'required' => FALSE,
        'maxlength' => 50,
      ),
      'company' => array(
        'type' => 'varchar',
        'title' => t('Company'),
        'maxlength' => 50,
        'required' => FALSE,
      ),
      'address' => array(
        'type' => 'address',
        'title' => t('Address'),
        'required' => FALSE,
      ),
    );
  }


  /**
   * Update the order status from Ammado
   */
  function update_order_status() {

    $status = $this->_ammado_widget_getOrderStatus($this->ammado->get_endpoint()
                                          , $this->ammado->get_api_key()
                                          , $this->ammado->get_order_id());
    if(is_array($status) && isset($status['http_status']))
      switch ($status['http_status']) {
        case 200:
        case 201:
          $this->ammado->ammado_update_order_status($status);
          break;
        case 404:
          $status['orderStatus'] = 'cancelled';
          $this->ammado->set_order_status('cancelled');
          $this->ammado->save();
          break;
        default:
          $status['orderStatus'] = 'error';
          $this->ammado->set_order_status('error');
          $this->ammado->save();
      }

    return $status;
  }

  /**
   * Preset Ammado widget with donor email, first name and last name.
   */
  function get_signed_widget( $email="ken+bixbender@causecast.org", $first_name="Bix", $last_name="Bender", $billing = "default" ) {


    if (isset($_REQUEST['ken_debug'])){
      $errstring = "\n\nEnter get_signed_widget with:\n"
      ."\nemail = ".var_export($email, true)."\n******\n"
      ."\nfirst_name = ".var_export($first_name, true)."\n******\n"
      ."\nlast_name = ".var_export($last_name, true)."\n******\n"
      ."\nbilling = ".var_export($billing, true)."\n******\n"
      ;
      watchdog("Ammado ", $errstring);

    }


    // Create the transaction
    // Create a brand new transaction object and set its pay method.
    $txn = tycoon_transaction_load();
    $pay_method = $txn->set_pay_method($this->payment_method_id );
    $txn->save();


    if (isset($_REQUEST['ken_debug'])){
      $errstring = "\n\nIn get_signed_widget with:\n"
      ."\ntxn = ".var_export($txn, true)."\n******\n"
      ;
      watchdog("Ammado ", $errstring);

    }


    // Load the ammado object using the transaction id.
    $this->ammado->load_by_txid($txn->txid);

    $bid = $this->ammado->get_billing_id();
    if ($bid != $billing){
      // We need to load another set of endpoint data if defined.
      $res = db_select('tycoon_cc_ammado_endpoints','tcae')
        ->fields('tcae')
        ->condition('tcae.billing_id',$billing)
        ->execute()->fetchAssoc();

       if ($res) {
        $this->ammado->set_endpoints_db_record($res);
        $this->ammado->ammado_init_from_endpoints_db_record();
       }
     }


     $widget_endpoint = $this->_ammado_widget_parseApiEndpointFromWidgetCode($this->ammado->get_widget_code());
     $order_data = $this->_ammado_widget_setupWidget($widget_endpoint, $this->ammado->get_api_key(), $email, $first_name, $last_name, $billing);
     $this->ammado->set_order_id($order_data['orderId']);
     $this->ammado->set_order_salt($order_data['salt']);
     $signature = $this->_ammado_widget_getOrderSignature();

      // append signature params to the widget code
     $signed_widget_code = $this->_ammado_widget_getSignedWidgetCode($this->ammado->get_widget_code(), $order_data['orderId'], $signature);

   // Set the first name, last name and email address
       // Set the object properties
        $txn->mail        = $email;
        $txn->first_name  = $first_name;
        $txn->last_name   = $last_name;

   // Save the transaction and the ammado record:
        $txn->save();
        $this->ammado->save();


     return $signed_widget_code ;

  }

/*******************************************************************************
 *  Functions from Ammado's API examples
 *******************************************************************************/
  private function _ammado_widget_parseWidgetUrlFromWidgetCode($widget_code)
  {
    if (!preg_match('/src=["\']([^"\']+)["\']/', $widget_code, $matched)) {
      throw new Exception("Unable to parse the Giving Widget code");
    }
  //
  return $matched[1];
  }

  private function _ammado_widget_parseApiEndpointFromWidgetCode($widget_code)
  {
    $widget_url = $this->_ammado_widget_parseWidgetUrlFromWidgetCode($widget_code);
    $parsed_url = parse_url($widget_url);
    //
    $api_host = str_replace('www.', 'api.', $parsed_url['host']);
    if (!preg_match('/^(\/.*\/givingwidget)(\/embed\.js)?$/U', $parsed_url['path'], $matched)) {
      throw new Exception("Unable to parse the Giving Widget URL");
    }
    //
    $api_endpoint = '/'.($this->ammado->get_api_version()).$matched[1];
    //
    return 'https://'.$api_host.$api_endpoint;
  }
  //

  private function _ammado_widget_setupWidget($endpoint, $apiKey, $donorEmail, $donorFirstName, $donorLastName, $extraInfo)
  {
    // prepare curl
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_VERBOSE, 0);
    //
    // setup request - POST to the endpoint with user data
    curl_setopt($ch, CURLOPT_POST, true);
    curl_setopt($ch, CURLOPT_URL, $endpoint);
    curl_setopt($ch, CURLOPT_POSTFIELDS, array(
      'apiKey' => $apiKey,
      'donorEmail' => $donorEmail,
      'donorFirstName' => $donorFirstName,
      'donorLastName' => $donorLastName,
      'extraInfo' => $extraInfo,
    ));
    //
    // execute request
    $result = curl_exec($ch);
    if ($result === false) {
      throw new Exception('Curl error: ' . curl_error($ch));
    }
    //
    $info = curl_getinfo($ch);
    $json = json_decode($result, true);
    $http_status = $info['http_code'];
    //
    // API returned an error
    if ($http_status != 201) {
      $error = 'unknown';
      if (!empty($json['errorCodes'])) {
        $codes=implode(', ', $json['errorCodes']);
      }
      throw new Exception('API returned errors: '.$error);
    }
    //
    // API returned success
    return $json;
    }
  //
  private function _ammado_widget_getOrderSignature()
  {
     return strtoupper(sha1(($this->ammado->get_order_id()).'.'.($this->ammado->get_order_salt()).'.'.($this->ammado->get_api_secret())));
  }
  //
  private function _ammado_widget_getSignedWidgetCode($widget_code, $orderId, $signature)
  {
    $widget_url = $this->_ammado_widget_parseWidgetUrlFromWidgetCode($widget_code);
    //
    $append  = (strpos($widget_url, '?') !== false ? '&' : '?');
    $append .= 'orderId=';
    $append .= urlencode($orderId);
    $append .= '&';
    $append .= 'signature=';
    $append .= urlencode($signature);
    //
    $modified_url = $widget_url.$append;
    return str_replace($widget_url, $modified_url, $widget_code);
  }

  private function _ammado_widget_getOrderStatus($endpoint, $apiKey, $orderId)
  {

      if (isset($_REQUEST['ken_debug'])){
        $err_string = "\n\nIn _ammado_widget_getOrderStatus with:"
        ."\n\n***\n\nendpoint:\n"
        ."\n".var_export($endpoint, true)."\n"
        ."\n\n***\n\napiKey:\n"
        ."\n".var_export($apiKey, true)."\n"
        ."\n\n***\n\norderId:\n"
        ."\n".var_export($orderId, true)."\n";

      }

        // prepare curl
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_VERBOSE, 0);
    //
    // setup request - POST to the endpoint with user data
    curl_setopt($ch, CURLOPT_POST, true);
    curl_setopt($ch, CURLOPT_URL, $endpoint."/order/".$orderId);


    if (isset($_REQUEST['ken_debug'])){
      $err_string= "url:\n"
      .$endpoint."order/".$orderId;
      watchdog('status',$err_string);

    }

    curl_setopt($ch, CURLOPT_POSTFIELDS, array(
      'apiKey' => $apiKey,
    ));
    //
    // execute request
    $result = curl_exec($ch);
    if ($result === false) {

     if (isset($_REQUEST['ken_debug'])){
        $err_string= "<br>Curl error:\n<br>"
        .vcurl_error($ch);
        watchdog('error',$err_string);
      }
     throw new Exception('Curl error: ' . curl_error($ch));

    }
    //
    $info = curl_getinfo($ch);
    $json = json_decode($result, true);
    $http_status = $info['http_code'];


    if (isset($_REQUEST['ken_debug'])){
      $err_string= "json:\n"
      .$result;
      watchdog('status',$err_string);

    }

    if (!is_array($json) ){

      if (isset($_REQUEST['ken_debug'])){
        $err_string= "<pre>\nResult not an array:\n"
        ."\nhttp_status:".$http_status."\n"
        ."</pre>";
        watchdog('status',$err_string);

      }

     }

     $json['http_status'] = $http_status ;


    //
    // API returned an error
    if ($http_status != 201) {
      $error = 'unknown';
      if (!empty($json['errorCodes'])) {
        $codes=implode(', ', $json['errorCodes']);
      }
    //			throw new Exception('API returned errors: '.$error);
    print "\n\n<BR/><BR/>API returned errors:\n<BR/>";
    print "<pre>".var_export($error, true)."</pre>";

    }
    //
    // API returned success
    return $json;
  }


}
