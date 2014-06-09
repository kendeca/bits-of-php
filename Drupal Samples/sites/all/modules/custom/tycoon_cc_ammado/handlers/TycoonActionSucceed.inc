<?php

class TycoonActionSucceed extends TycoonActionHandler {

  protected $key = 'succeed';

  function execute(&$args) {
  //This will be called when an ammado succeed state is identified.
      if (isset($_REQUEST['ken_debug'])){
        $err_msg = "In execute in TycoonActionSucceed\n\n"
        ."\n\nargs = ".var_export($args, 1)
        ."\n\nthis = ".var_export($this, 1)
        ;
        watchdog('ken_debug',$err_msg);

      }

    try {
      $pay_method = $this->txn->pay_method();
      $data = $pay_method->succeed($this->txn, $args);
      if ($data === TRUE) {
        $data = array();
      }
      $data['pay_method_class'] = get_class($pay_method);

      $this->set_status(TYCOON_ACTION_SUCCESS);
      $activity = array('message' => 'Succeed successful.', 'data' => $data);
      $aid = $this->log_activity($activity + array('args' => array(), 'data' => array()));
    }
    catch (TycoonActionException $e) {
      $this->set_status(TYCOON_ACTION_FAILED);
      $this->set_errors($e->errors);
      $aid = $this->log_activity($e->activity());
    }
    // $pay_method->save($aid, $data);
    $this->aid = $aid;
    return $aid;
  }

  static function action_info($activity) {
    return call_user_func(array($activity->data['pay_method_class'], 'format_action'), $activity->aid, $activity->data);
  }
}
