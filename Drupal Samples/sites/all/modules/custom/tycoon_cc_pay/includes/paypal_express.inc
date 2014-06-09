<?php
/**
 * @file
 * Definition of TycoonCCPayPalExpressMethod
 */

/**
  * Class for PayPal Express payments
  *
  * TycoonCCPayPalExpressMethod is the class for the employer match pay method.
  *
  */
class TycoonCCPayPalExpressMethod extends TycoonCCPayPalMethod {

 /**
   * Constant to make the id of this class easy to get
   */
  const pmid = 'tycoon_cc_pay_paypal_express';

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
    //@todo Validate functions specific to Express payment
    // The validate process will require us to go to paypal, come back, and process the payment.

  if (isset($_REQUEST['ken_debug'])) {
      $err_msg = "\n\nIn validate in paypal_express.inc\n\n"
                ."\n\n******\n\nrecur = ".((isset($transaction->recur) && is_array($transaction->recur))?var_export($transaction->recur, 1):" RECUR not set")
                ."\n\n******\n\none_click = ".((isset($transaction->one_click) && is_array($transaction->one_click))?var_export($transaction->one_click, 1):" ONE_CLICK not set")
                ."\n\n******\n\nTransaction = ".var_export($transaction, 1)
                ."\n\n******\n\nData = ".var_export($data, 1)
                ."\n\n******\n\nthis = ".var_export($this, 1);
      watchdog('ken_debug',$err_msg);
    }
// Load the paypal object with the transaction.

// Check recurring, Oneclick, client_covers_fees, employer_match.

    $flavor = 'express';

    if(isset($transaction->recur) && is_array($transaction->recur)){
        // This is the first of a transaction that will recur.  It is a special
        // flavor of the express transaction.
        $flavor = 'reference';
        $recur_array = $transaction->recur;
        $recur_array['recur'] = true;
      } else {
        // Plain old express transaction
        $recur_array = array();
      }

    if((isset($transaction->one_click) && is_array($transaction->one_click))){
        // This will define a one_click payment record.  It is a special
        // flavor of the express transaction.
        $flavor = 'reference';
        $one_click_array = $transaction->one_click;
        $one_click_array['one_click'] = true;
      } else {
        // Plain old express transaction
        $one_click_array = array();
      }

    if((isset($transaction->client_covers_fees) )&& ($transaction->client_covers_fees)){
        // Client is covering fees.  The flavor can be either.
        $client_covers_fees_array = array('client_covers_fees'=>true);
      } else {
        // Plain old express transaction
        $client_covers_fees_array = array();
      }

    if((isset($transaction->employer_match) )&& ($transaction->employer_match)){
        // Client is matching.  The flavor can be either.
        $employer_match_array = array('employer_match'=>true
            ,'billing_id'=>($transaction->billing_id));
      } else {
        // Plain old express transaction
        $employer_match_array = array();
      }

    $this->paypal_express_load_paypal($transaction->txid, $flavor );

    if($this->paypal){
    // Do we have a token?
        $token = $this->paypal->get_token();
        if (empty($token)){
        //  We need to set up the Express Checkout and then transfer them to Paypal

        $serverName = $_SERVER['SERVER_NAME'];
        $serverPort = $_SERVER['SERVER_PORT'];
        $url = 'http://'.$serverName.':'.$serverPort;

        $returnlink = $url.'/cc_process_order.php?'.(isset($_REQUEST['ken_debug'])?'ken_debug=true&':'').'txid='.($transaction->txid)
        				."&flavor=".$flavor;
        $cancellink = $url.$_SERVER['REQUEST_URI'];  // Use this link for cancellation if no additional processing needded.



        $this->paypal->set_np_name($transaction->np->title);
        $this->paypal->set_donation_amount($transaction->total);
        $this->paypal->set_recur($recur_array);
        $this->paypal->set_one_click($one_click_array);
        $this->paypal->set_client_covers_fees($client_covers_fees_array);
        $this->paypal->set_employer_match($employer_match_array);


        // Set up the return URLs
        // Make sure we pass back the txid and token on the query string.
        $this->paypal->set_return_url(urlencode($returnlink.'&cc_status=valid'));
//        $this->paypal->set_cancel_url(urlencode($returnlink.'&cc_status=cancel')); // This version allows processing on page
        $this->paypal->set_cancel_url(urlencode($cancellink));


        $this->paypal->set_express_checkout();


        // Now construct the linkback link.

            global $user;

            $query = array(
              'transaction_id' => $transaction->txid,
              'session_key' => isset($data['session_key']) ? $data['session_key'] : NULL,
              'flavor' => $flavor
            );
            if (isset($data['spddest'])) {
              if (!empty($data['spddest'] ))  $query['spddest'] = $data['spddest'];
            }

              if (!empty($data['post_back_url'])) {
                // if post back has any GET params, we need to strip those out before rebuilding url
                $url_data = drupal_parse_url($data['post_back_url']);
                $query += $url_data['query'];
                $linkback = $url_data['path'] . '?' . http_build_query($query);
              } else {
                $linkback = "";
              }

            //Route users to thank you page unless linkback is set or user is
            //from ee
          if (!ee_from_ee() && empty($linkback)) {
            $linkback = $url.'/thank-you/?npid='. ($transaction->np->nid) . ((!empty($linkback))?('&dest=' . urlencode($linkback)):"");
          }


      if (isset($_REQUEST['ken_debug'])) {
            $err_msg = "In paypal_express:validate before going to paypal. \n\n"
                      .'\n\n\$this->paypal = '.var_export($this->paypal, 1)
                      .'\n\n\$linkback = '.var_export($linkback, 1)
                     .'\n\n\$transaction = '.var_export($transaction, 1);
            watchdog('ken_debug',$err_msg);
          }

         $this->paypal->set_linkback($linkback);

        // set form code so that we can retrieve the cached data when the user
        // gets back from paypal
        if (!empty($_GET['form_gen'])) {
          $this->paypal->set_form_gen($_GET['form_gen']);
        }


      // Save the affiliate
      if(module_exists('cc_sitekey')){
            _cc_sitekey_form_affiliate_log($transaction, $transaction->affiliate);
            }
      // Save the transaction because we are going away.
        $transaction->save();

        // Now scram!
        $this->paypal->go_to_paypal();

      } else {
        //  We must be coming back from the paypal validated, so validate this transaction
        //  And validate the details
       if (isset($_REQUEST['ken_debug'])) {
          $err_msg = "In validate in paypal_express.inc after returning from paypal.\n\n"
                    .'\n\n\$this->paypal = '.var_export($this->paypal, 1)
                    ;
          watchdog('ken_debug',$err_msg);
        }
       $this->paypal->get_express_checkout_details();

        $this->paypal->save();  // Save the details

       // Update total_paid with the total paid by paypal
       $transaction->total_paid = $this->paypal->get_total_paid();


        return TRUE;
        }
    } else
    {
      return FALSE;
    }

  }

  function process(&$transaction, &$data) {
    if (isset($_REQUEST['ken_debug'])) {
        $err_msg = "\n\nIn process in paypal_express.inc\n\n"
                ."\n\nTransaction = ".var_export($transaction, 1)
                ."\n\nData = ".var_export($data, 1)
                ."\n\nrecur = ".(isset($transaction->recur)?var_export($transaction->recur, 1):" RECUR not set")
                ."\n\none_click = ".(isset($transaction->one_click)?var_export($transaction->one_click, 1):" ONE_CLICK not set")
                ."\n\npay_method = ".(isset($transaction->pay_method)?var_export($transaction->pay_method, 1):" PAY_METHOD not set")
                ."\n\nthis = ".var_export($this, 1);
      watchdog('ken_debug',$err_msg);
    }

    parent::process($transaction, $data);

// Load the paypal object with the transaction.

    $flavor = 'express';
    $recurring = !empty($transaction->recur);
    $one_click = !empty($transaction->one_click);
    $employer_match = !empty($transaction->one_click_array);


    if($recurring || $one_click ){
        // This is the first of a transaction that will recur.  It is a special
        // flavor of the express transaction.
        $flavor = 'reference';
      }


    $this->paypal_express_load_paypal($transaction->txid, $flavor );

    // Make sure that we've been set, authorized, have all details.

    // If so, get the details and do the transaction!

    $this->paypal->get_express_checkout_details();

     // Grab the name and email address from the checkout.
    if (!isset($transaction->mail) || empty($transaction->mail)) $transaction->mail = $this->paypal->get_payer_email();
    if (!isset($transaction->first_name) || empty($transaction->first_name)) $transaction->first_name = $this->paypal->get_payer_first();
    if (!isset($transaction->last_name) || empty($transaction->last_name)) $transaction->last_name = $this->paypal->get_payer_last();

    $this->paypal->do_express_checkout();

    //we need to get the correct gateway fee from paypal and update the
    //transaction with it.
    $transaction->update_gateway_fee($this->paypal->get_paypal_fee());

    // Update recurring or one_click data.

    if ($recurring) $transaction->recur = $this->paypal->get_recur();
    if ($one_click) $transaction->one_click = $this->paypal->get_one_click();

    $client_covers_fees_array = $this->paypal->get_client_covers_fees();
    if (!empty($client_covers_fees_array)
          && is_array($client_covers_fees_array)) {
          $client_covers_fees_array = $this->paypal->get_client_covers_fees();
          if (isset($client_covers_fees_array['client_covers_fees'])){
            $transaction->client_covers_fees = $client_covers_fees_array['client_covers_fees'];
            $transaction->client_covers_fees_array = $client_covers_fees_array;
            }
     }

    $employer_match_array = $this->paypal->get_employer_match();
    if (!empty($employer_match_array)
          && is_array($employer_match_array)) {
          $employer_match_array = $this->paypal->get_employer_match();
          if (isset($employer_match_array['employer_match'])){
            $transaction->employer_match = $employer_match_array['employer_match'];
            $transaction->billing_id = $employer_match_array['billing_id'];
            $transaction->employer_match_array = $employer_match_array;
            $transaction->pay_method->billing_id = $employer_match_array['billing_id'];
            }
      }

    $this->paypal->save();

    //We may need

    return $data;
  }

  function settle(&$transaction, &$data) {
    parent::settle($transaction, $data);
    //This will be called to settle things when the transaction settles.
    if (isset($_REQUEST['ken_debug'])) {
      $err_msg = "In settle in paypal_express.inc\n\n"
                   ."\n\nTransaction = ".var_export($transaction, 1)
                  ."\n\nData = ".var_export($data, 1)
                  ."\n\nrecur = ".(isset($transaction->recur)?var_export($transaction->recur, 1):" RECUR not set")
                  ."\n\nthis = ".var_export($this, 1);
      watchdog('ken_debug',$err_msg);
    }


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


  //@todo modify this schema for Paypal express payments
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

  function options_form(&$form, &$form_state) {
    $form['test'] = array(
      '#type' => 'checkbox',
      '#title' => t('Test mode'),
      '#default_value' => !empty($this->options['test']),
    );
    $form['sandbox'] = array(
      '#type' => 'checkbox',
      '#title' => t('Sandbox mode'),
      '#default_value' => !empty($this->options['sandbox']),
    );
    $form['transaction_fee_percent'] = array(
      '#type' => 'textfield',
      '#title' => 'Variable transaction fee (decimal fraction)',
      '#default_value' => isset($this->options['transaction_fee_percent'])?($this->options['transaction_fee_percent']):'.029',
    );
    $form['transaction_fee_fixed'] = array(
      '#type' => 'textfield',
      '#title' => 'Fixed transaction fee (USD)',
      '#default_value' => isset($this->options['transaction_fee_fixed'])?($this->options['transaction_fee_fixed']):'0.30',
    );
    $form['render_form_elements'] = array(
      '#type' => 'checkbox',
      '#title' => t('Render form elements'),
      '#default_value' => !empty($this->options['render_form_elements']),
    );
    $form['radio_button_label'] = array(
      '#type' => 'textfield',
      '#title' => t('Radio Button Label'),
      '#default_value' => $this->options['radio_button_label'],
    );

    $form['paypal_environment'] = array(
      '#type' => 'textfield',
      '#title' => t('Paypal environment'),
      '#default_value' => !empty($this->options['paypal_environment'])?($this->options['paypal_environment']):variable_get('cc_pp_environment','sandbox'),
    );

   // Need to hide value of paypal_username for security reasons.
    $form['paypal_username'] = array(
      '#type' => 'value',
      '#value' => !empty($this->options['paypal_username'])?($this->options['paypal_username']):variable_get('cc_pp_username','ccpay_1343168213_biz_api1.gmail.com'),
    );
    $form['paypal_username_input'] = array(
      '#type' => 'textfield',
      '#title' => t('Paypal username'),
    );
    $form['paypal_username_change'] = array(
      '#type' => 'checkbox',
      '#title' => t('Change'),
      '#description' => t('Check this box if you would like to change the Paypal username. If left unchecked, the Paypal username will remain unchanged.'),
      '#default_value' => 0,
    );


   // Need to hide value of paypal_password for security reasons.
    $form['paypal_password'] = array(
      '#type' => 'value',
      '#value' => !empty($this->options['paypal_password'])?($this->options['paypal_password']):variable_get('cc_pp_password','1343168236'),
    );
    $form['paypal_password_input'] = array(
      '#type' => 'textfield',
      '#title' => t('Paypal password'),
    );
    $form['paypal_password_change'] = array(
      '#type' => 'checkbox',
      '#title' => t('Change'),
      '#description' => t('Check this box if you would like to change the Paypal password. If left unchecked, the Paypal password will remain unchanged.'),
      '#default_value' => 0,
    );


   // Need to hide value of paypal_signature for security reasons.
    $form['paypal_signature'] = array(
      '#type' => 'value',
      '#value' => !empty($this->options['paypal_signature'])?($this->options['paypal_signature']):variable_get('cc_pp_signature','AalEHdhy4S72paa-t6Wa6UZpX75AAeJbsur.xQNHsA3Vb.QwU1SreRTO'),
    );
    $form['paypal_signature_input'] = array(
      '#type' => 'textfield',
      '#title' => t('Paypal signature'),
    );
    $form['paypal_signature_change'] = array(
      '#type' => 'checkbox',
      '#title' => t('Change'),
      '#description' => t('Check this box if you would like to change the Paypal signature. If left unchecked, the Paypal signature will remain unchanged.'),
      '#default_value' => 0,
    );
    $form['paypal_endpoint'] = array(
      '#type' => 'textfield',
      '#title' => t('Paypal NVP API Endpoint'),
      '#default_value' => !empty($this->options['paypal_endpoint'])?($this->options['paypal_endpoint']):variable_get('cc_pp_endpoint','https://api-3t.sandbox.paypal.com/nvp'),
    );
 // Sandbox
    $form['paypal_version'] = array(
      '#type' => 'textfield',
      '#title' => t('Paypal version'),
      '#default_value' => !empty($this->options['paypal_version'])?($this->options['paypal_version']):variable_get('cc_pp_version','92.0'),
    );

    $form['paypal_paypalurl'] = array(
      '#type' => 'textfield',
      '#title' => t('Paypal URL'),
      '#size' => 72,
      '#default_value' => !empty($this->options['paypal_paypalurl'])?($this->options['paypal_paypalurl']):variable_get('cc_pp_paypalurl','https://www.sandbox.paypal.com/webscr'),
    );
    }

  /**
   * Validate the options form.
   */
  function options_validate(&$form, &$form_state) {
     $values = &$form_state['values'];


    if (!empty($values['paypal_username_change'])) {
      $values['paypal_username'] = $values['paypal_username_input'];
    }

    if (!empty($values['paypal_password_change'])) {
      $values['paypal_password'] = $values['paypal_password_input'];
    }

    if (!empty($values['paypal_signature_change'])) {
      $values['paypal_signature'] = $values['paypal_signature_input'];
    }

    if (!empty($values['transaction_fee_percent']) && !is_numeric($values['transaction_fee_percent'])) {
      form_set_error('transaction_fee_percent', t('Variable transaction fee must be a number.'));
    }

    if (!empty($values['transaction_fee_fixed']) && !is_numeric($values['transaction_fee_fixed']) ){
      form_set_error('transaction_fee_fixed', t('Fixed transaction fee must be a number.'));
    }

    }
  /**
   * Load the Paypal object for the specified transaction.
   *
   * @param txid
   *   The transaction for which the paypal object is to be loaded.
   * @return
   *   The paypal object if the load successful or false if the load failed.
   */
  function paypal_express_load_paypal($txid,$flavor='express') {

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
  }


}
