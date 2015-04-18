function collapse(divId, iconId) {
  var classes = document.getElementById(divId).className;
  var collapseClass = "ui-collapsible-content-collapsed";
  if (-1 === classes.indexOf(collapseClass)) {
    document.getElementById(divId).className += " "+collapseClass;
    switchIcon(iconId, true);
  }
  else {
    document.getElementById(divId).className = classes.replace(collapseClass, "");
    switchIcon(iconId, false);
  }
}

function switchIcon(elementId, showIconDown) {
  var classes = document.getElementById(elementId).className;
  if (true == showIconDown) {
    classes = classes.replace("ui-icon-carat-u", "ui-icon-carat-d");
  }
  else {
    classes = classes.replace("ui-icon-carat-d", "ui-icon-carat-u");
  }
  document.getElementById(elementId).className = classes;
}
