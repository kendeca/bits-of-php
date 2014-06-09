<?php
/**
 * @file
 *   Class for handling some of CC webapi functionality
 */
class AmmadoTycoon extends TycoonWebAPI {

  // Ammado widget object
  protected $widget_object ;



  /**
   * Get Ammado donation button
   */
  function get_international_donate_button($post)
  {
      if (isset($_REQUEST['ken_debug'])){
        watchdog('international 003','Enter get_international_donate_button with post = '.var_export($post, true));
      }
   $this->widget_object = new TycoonCCAmmadoWidgetMethod();
    $this->widget_object->init(array());

    $billing_id = isset($post['billing_id'])?$post['billing_id']:"default"; // Billing id associated with this
    $first_name = isset($post['first_name'])?$post['first_name']:""; // Donor’s first name
    $last_name = isset($post['last_name'])?$post['last_name']:""; // Donor’s last name
    $mail = isset($post['mail'])?$post['mail']:"email missing"; // Donor’s email address.

    // The email address is required (but only the email address).
    if ($mail == "email missing"){
          throw new TycoonWebAPIException("Email address required", 'getInternalDonateButton', 404);
    }

    $embed_code = $this->widget_object->get_signed_widget( $mail, $first_name, $last_name, $billing_id);
    $return_array = array("widget_code"=>$embed_code);
      if (isset($_REQUEST['ken_debug'])){
        watchdog('international 004','In get_international_donate_button with embed code = '.var_export($embed_code, true));
        watchdog('international 005','In get_international_donate_button with return array = '.var_export($return_array, true));
      }
    return (array)$return_array ;
  }

}

