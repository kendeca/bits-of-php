<?php
/**
 * @file
 * Definition of TycoonCCPayPalReferenceMethod
 */

/**
  * Class for PayPal Reference payments
  *
  * TycoonCCPayPalReferenceMethod is the class for the Paypal reference
  * transaction payment method.
  *
  */
class TycoonCCPayPalReferenceMethod extends TycoonCCPayPalMethod {

 /**
   * Constant to make the id of this class easy to get
   */
  const pmid = 'tycoon_cc_pay_paypal_reference';

  /**
   *
   * The Paypal object supporting this Express Checkout object
   */

  public $paypal ;

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
    module_load_include('inc', 'tycoon_cc_pay', 'tycoon_cc_pay');


  }

  function validate(&$transaction, &$data) {
    parent::validate($transaction, $data);
    //@todo Validate functions specific to Reference payment

    // Debugging reference
       $err_msg = "\n\n****\n\nIn validate in paypal_reference.inc\n\n****\n\n"
              ."\n\n****\n\nTransaction = ".var_export($transaction, 1)
              ."\n\n****\n\nData = ".var_export($data, 1)
              ."\n\n****\n\ndata['billing_id'] = "
                .(isset($data['billing_id'])?var_export($data['billing_id'], 1):" billing_id not set")
              ."\n\n****\n\nthis = ".var_export($this, 1);
    watchdog('reference_debug',$err_msg);

   // Make sure that the paypal object is for a user with a pre-authorized
   // billing id.
   $flavor = 'reference';
   $ret = true;

   $this->paypal_reference_load_paypal($transaction->txid, $flavor );

   // If this is from a saved id, the billing agreement id is in the data.

   if (isset($data['billing_id'])) $this->paypal->set_billing_agreement_id($data['billing_id']);

   $bag = $this->paypal->get_billing_agreement_id();
   $ret = (strlen($bag) > 0);

   $this->paypal->save();

    return $ret;
  }

  function process(&$transaction, &$data) {
    parent::process($transaction, $data);
    //@todo process operations specific to Reference payment
  // Debugging reference
       $err_msg = "\n\n****\n\nIn process in paypal_reference.inc\n\n****\n\n"
              ."\n\n****\n\nTransaction = ".var_export($transaction, 1)
              ."\n\n****\n\nData = ".var_export($data, 1)
              ."\n\n****\n\nrecur = ".(isset($transaction->recur)?var_export($transaction->recur, 1):" RECUR not set")
              ."\n\n****\n\ndata['billing_id'] = ".(isset($data['billing_id'])?var_export($data['billing_id'], 1):" billing_id not set")
              ."\n\n****\n\nthis = ".var_export($this, 1);
    watchdog('reference_debug',$err_msg);


    // Load the paypal object with the transaction.

    $flavor = 'reference';


    $this->paypal_reference_load_paypal($transaction->txid, $flavor );

     // Grab the name and email address from the checkout.
    if (!isset($transaction->mail) || empty($transaction->mail)) $transaction->mail = $this->paypal->get_payer_email();
    if (!isset($transaction->first_name) || empty($transaction->first_name)) $transaction->first_name = $this->paypal->get_payer_first();
    if (!isset($transaction->last_name) || empty($transaction->last_name)) $transaction->last_name = $this->paypal->get_payer_last();

    $this->paypal->set_donation_amount($transaction->total);
    if (isset($data['currency_code']) )$this->paypal->set_currency_code($data['currency_code']);
    $this->paypal->do_reference_transaction();

    //we need to get the correct gateway fee from paypal and update the
    //transaction with it.
    $transaction->update_gateway_fee($this->paypal->get_paypal_fee());

    $this->paypal->save();

     // If the state is not pending, we can tell calling program to call settle!
      if (strtoupper($this->paypal->get_payment_status()) == 'COMPLETED'){
        // signal that we can the transaction!
        $data['can_settle']=true;
      } else $data['can_settle']=false;

    return $data;
  }

  function settle(&$transaction, &$data) {
    parent::settle($transaction, $data);
    //@todo settle operations specific to Reference payment
 // Debugging reference
       $err_msg = "\n\nIn settle in paypal_reference.inc\n\n"
              ."\n\nTransaction = ".var_export($transaction, 1)
              ."\n\nData = ".var_export($data, 1)
              ."\n\nrecur = ".(isset($transaction->recur)?var_export($transaction->recur, 1):" RECUR not set")
              ."\n\nthis = ".var_export($this, 1);
    watchdog('reference_debug',$err_msg);
    return $data;
  }


  function tokens($tokens, $options = array()) {
    $replacements = array();
    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'name':
          $replacements[$original] = t('Paypal', array(), $options);
          break;
      }
    }
    return $replacements;
  }



  //@todo modify this schema for Paypal reference payments
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
    );
  }

  /**
   * Information about options used by the TycoonCCPayMethod class
   *
   * @return
   *  An associative array with the options as indexes and an arrays with
   *  information about each option as the values.
   */

  protected function option_definition() {
    $options = parent::option_definition();
    $options['test'] = array('default' => TRUE);
    $options['sandbox'] = array('default' => FALSE);

    // portion of per-transaction fee that is a fraction
    $options['transaction_fee_percent'] = array('default' => .029);

    // portion of per-transaction fee that is fixed per transaction in dollars
    $options['transaction_fee_fixed'] = array('default' => 0.30);

    $options['render_form_elements'] = array('default' => FALSE);
    $options['radio_button_label'] = array('default' => '');
    $options['paypal_environment'] = array('default' => variable_get('cc_pp_environment','sandbox'));
    $options['paypal_username'] = array('default' => variable_get('cc_pp_username','ccpay_1343168213_biz_api1.gmail.com'));
    $options['paypal_password'] = array('default' => variable_get('cc_pp_password','1343168236'));
    $options['paypal_signature'] = array('default' => variable_get('cc_pp_signature','AalEHdhy4S72paa-t6Wa6UZpX75AAeJbsur.xQNHsA3Vb.QwU1SreRTO'));
    $options['paypal_endpoint'] = array('default' => variable_get('cc_pp_endpoint','https://api-3t.sandbox.paypal.com/nvp'));
    $options['paypal_version'] = array('default' => variable_get('cc_pp_version','92.0'));
    $options['paypal_paypalurl'] = array('default' => variable_get('cc_pp_paypalurl','https://www.sandbox.paypal.com/webscr'));
    return $options;
  }


  /**
   * Load the Paypal object for the specified transaction.
   *
   * @param txid
   *   The transaction for which the paypal object is to be loaded.
   * @return
   *   The paypal object if the load successful or false if the load failed.
   */
  function paypal_reference_load_paypal($txid,$flavor='reference') {

    if (!is_object($this->paypal)) {
      $this->paypal = new TycoonCCPaypal();
      // Use the options to set up the paypal object
      $this->paypal->set_environment($this->options['paypal_environment']);
      $this->paypal->set_username($this->options['paypal_username']);
      $this->paypal->set_password($this->options['paypal_password']);
      $this->paypal->set_signature($this->options['paypal_signature']);
      $this->paypal->set_endpoint($this->options['paypal_endpoint']);
      $this->paypal->set_version($this->options['paypal_version']);
      $this->paypal->set_paypalurl($this->options['paypal_paypalurl']);
      $this->paypal->set_flavor($flavor);

    }


    return $this->paypal->load_by_txid($txid);

    // Since this is a reference transaction, we need to have the billing id set
    // somehow  --- Mustdo!!!
  }


}
