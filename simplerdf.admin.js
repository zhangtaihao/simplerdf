// $Id$
/**
 * Administration UI dynamic behavior implementation.
 */

$(document).ready(function () {
  // Node URI generator choice page
  var shown = false;
  $('.simplerdf-nodeuri-button:radio').change(function () {
    if (!shown) {
      // Show "changed" message on changing options
      $('#simplerdf-changed-message:not(.changed)').show(300);
      shown = true;
    }
  });
});