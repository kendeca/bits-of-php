<?php
/**
 * @file
 * This file is the customized Causecast invoice template.
 */
?>
<p style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;font-size:30px;margin:35px 50px;"><?php print t('Your Payroll Deduction Confirmation'); ?></p>
<p style="height:10px;width:100%;background-color:#02B9CB">&nbsp;</p>
<table width="500" style="margin:0px 50px;" border="0" cellspacing="0" cellpadding="0" align="center">
  <tr>
     <td>
       <table width="100%" border="0" cellspacing="0" cellpadding="0" align="left">
        <tr>
          <td style="padding: 10px 0;" colspan="2" align="left">
            <p style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;"><?php print t("Hi "); ?> [tycoon_transaction:first-name],</p>
            <p style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;"><?php print t("Thank you for your donation to "); print t(!empty($transaction->np->title) ? $transaction->np->title : ''); ?>!</b></p>
            <p style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;"><?php print t("This email is not your donation receipt - rather a confirmation that processing has begun on your request to deduct funds from your paycheck for the benefit of the nonprofit noted above."); ?></p>
            <p style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;"><?php print t("This functionality is a part of the "); print t(variable_get('site_name')); print t(" platform and is powered by Causecast. As such, ‘Causecast’ may appear on paystubs and in additional confirmation emails. "); ?></p>
            <p style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;"><?php print t("The process of distributing funds from your paycheck to your selected nonprofit may take up to 120 days. Once funds are on their way to the nonprofit a donation receipt will be sent to the email address provided. "); ?></p>
            <p style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;"><?php print t("The details of your donation are as follows: "); ?></p>
          </td>
        </tr>
        <tr>
          <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding: 5px 0 10px;" colspan="2" align="left">
            <table width="100%" border="0" cellspacing="0" cellpadding="1">
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;" width="30%">
                  <b><?php print('Nonprofit Organization:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  <?php print t($np->title); ?>
                </td>
              </tr>
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;">
                  <b><?php print t('Date:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  [tycoon_transaction:created]
                </td>
              </tr>
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;">
                  <b><?php print t('ID:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  [tycoon_transaction:txid]
                </td>
              </tr>
              <?php if (!empty($next_donation_date)): ?>
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;">
                  <b><?php print t('Type:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  <?php print t('Recurring Donation'); ?>
                </td>
              </tr>
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;">
                  <b><?php print t('Next Donation:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  <?php print $next_donation_date; ?>
                </td>
              </tr>
              <?php endif; ?>
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;">
                  <b><?php print t('Donor Information:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  <?php if (!empty($transaction->first_name) || !empty($transaction->last_name)): ?>
                  [tycoon_transaction:full-name] <br />
                  <?php endif; ?>
                  <?php if (!empty($transaction->company)): ?>
                  [tycoon_transaction:company] <br />
                  <?php endif; ?>
                  <?php if (!empty($transaction->postal_code) || !empty($transaction->street1)): ?>
                  [tycoon_transaction:address]
                  <?php endif; ?>
                </td>
              </tr>
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;">
                  <b><?php print t('Payment Method:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  [tycoon_pay_method:name]
                </td>
              </tr>
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;">
                  <b><?php print t('Donation Amount:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  $<?php print $amount; ?>
                </td>
              </tr>
              <?php //if (!$transaction->client_covers_fees) { // don't print fees if client covers them
                      if (false) { // don't print fees ever
                ?>
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;">
                  <b><?php print t('Transaction Fees:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  $<?php print $fees; ?>
                </td>
              </tr>
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;">
                  <b><?php print t('Total:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  $[tycoon_transaction:total]
                </td>
              </tr>
              <?php }// don't print fees,total if client covers them
                ?>
              <tr valign="top">
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 0;">
                  <b><?php print t('Tax-Deductible Amount:'); ?></b>
                </td>
                <td style="color:#888888;font-family:Lucida Grande,Lucida Sans,Lucida Sans Unicode,Helvetica,Arial,sans-serif;padding:6px 15px;">
                  $<?php print $total_notax; ?>
                </td>
              </tr>
            </table>
          </td>
        </tr>
        <?php if (!empty($employer_match)): ?>
        <tr>
          <td style="color:#888888;padding: 10px 0;"><b><?php print t('We are sending your employer the information needed to make a matching donation.'); ?></b></td>
        </tr>
        <?php endif; ?>
        <tr>
          <td style="color:#888888;padding: 0 0 10px;" align="left">
            <?php print t("<p><b>In Cause,</b><br /><b>Causecast</b>"); ?><br /><a href="http://causecast.com">www.causecast.com</a></p>
          </td>
        </tr>
      </table>
    </td>
  </tr>
  <tr>
    <td align="left">
      <table width="100%" border="0" cellspacing="0" cellpadding="0" align="left">
        <tr>
          <td style="color:#888888;border-top:1px solid #CCCCCC;padding-top:10px;font-size:small;"><?php print t("Please note that all donations are non-refundable. Should you have any further questions or issues, please e-mail us at !mail.", array('!mail' => '<a href="mailto:donations@cauecast.org">donations@causecast.org</a>')); ?></td>
        </tr>
      </table>
    </td>
  </tr>
</table>
