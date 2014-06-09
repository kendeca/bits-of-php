<?php
/**
 * @file
 * Functions and abstract classes for all Causecast pay method activities.
 */

/**
  * Parent class for Ammado paid methods
  *
  * TycoonCCAmmadoMethod is the parent class for any pay method that requires
  *   Causecast to use Ammado to cover a transaction.
  *
  */
abstract class TycoonCCAmmadoMethod extends TycoonCCPayMethod {


 /**
   * Property to make the id of this class easy to get
   */

   var $id = '';

  /**
   *
   * The Ammado object supporting this Ammado Widget object
   */

  public $ammado ;

  /**
   * Constructor class.  Set the id.
   */
  function __construct($options=array()) {
    parent::__construct($options);

  }


  /**
   * Initializes the class
   */
  function init($options) {
    parent::init($options);
    module_load_include('inc', 'tycoon_cc_ammado', 'tycoon_cc_ammado');

    $this->ammado = new TycoonCCAmmado();

  }


  /**
   * Validates fields.
   *
   * @param $transaction
   *   The transaction object.
   * @param $data
   *   For employer payment methods
   *    all we need is a non-empty billing_id string.
   *
   * @return
   *   TRUE on success.
   *   Array of errors on failure.
   */
  function validate(&$transaction, &$data) {
    parent::validate($transaction,$data);
    // @todo Put common validation action code here.
    return TRUE;
  }


  function process(&$transaction, &$data) {

    if (isset($_REQUEST['ken_debug'])){
      $err_msg = "In process in tycoon_cc_ammado.inc\n\n"
      ."\n\nTransaction = ".var_export($transaction, 1)
      ."\n\nData = ".var_export($data, 1)
      ."\n\nthis = ".var_export($this, 1);
      watchdog('ken_debug',$err_msg);

    }

    try {
      parent::process($transaction, $data);
    }
    catch (TycoonActionException $e) {
      throw new TycoonActionException('Ammado payment transaction failed.', $e->args, $e->errors, $e->getCode());
    }
    //@todo Implement Ammado common process actions
    $transaction->total_paid = $this->ammado->get_donation_amount();
    $transaction->total = $this->ammado->get_donation_amount();
    $transaction->currency = $this->ammado->get_currency_code();


    return $data;
  }

  function succeed(&$transaction, &$data) {
    //This will be called when an ammado succeed state is identified.

    if (isset($_REQUEST['ken_debug'])){
      $err_msg = "In succeed in tycoon_cc_ammado.inc\n\n"
      ."\n\nTransaction = ".var_export($transaction, 1)
      ."\n\nData = ".var_export($data, 1)
      ."\n\nrecur = ".(isset($transaction->recur)?var_export($transaction->recur, 1):" RECUR not set")
      ."\n\nthis = ".var_export($this, 1);
      watchdog('ken_debug',$err_msg);

    }


    $transaction->total_paid = $this->ammado->get_donation_amount();
    $transaction->total = $this->ammado->get_donation_amount();
    $transaction->currency = $this->ammado->get_currency_code();


    return $data;
  }


/**
 * Perform the settle action on this transaction
 *
 */
  function settle(&$transaction, &$data) {

    //@todo Implement Ammado common settle actions
    return $data;
  }

/**
 * Perform the cancel action on this transaction
 *
 */
  function cancel(&$transaction, &$data) {


      if (isset($_REQUEST['ken_debug'])){
        $err_msg = "In cancel in tycoon_cc_ammado.inc\n\n"
        ."\n\nTransaction = ".var_export($transaction, 1)
        ."\n\nData = ".var_export($data, 1)
        ."\n\nthis = ".var_export($this, 1);
        watchdog('ken_debug',$err_msg);

      }


      $this->ammado->set_order_status('cancelled');
      $this->ammado->save();
      $transaction->set_state('canceled');



      parent::cancel($transaction,$data);


    return $data;
  }

  function save($aid,$data) {
    parent::save($aid,$data);
  }
}


?>
