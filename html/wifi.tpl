<!DOCTYPE html>
<html class="ui-mobile">
<head>
<title>JADJA</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" href="jquery.mobile-1.4.5.min.css">
<link rel="stylesheet" href="style.css">
<script type="text/javascript" src="140medley.min.js"></script>
<script type="text/javascript" src="common.js"></script>
<script type="text/javascript">

var xhr=j();
var currAp="%currSsid%";

var networks=[];

function createDivider() {
  var divider=document.createElement("li");
  divider.className="ui-li-divider ui-bar-inherit ui-first-child";
  divider.textContent="Please select your WiFi network:";
  return divider;
}

function clearSelectedItem() {
  for(var nIter=0;nIter<networks.length;nIter++) {
    var linkElement = document.getElementById("link"+networks[nIter]);
    if (null !== linkElement) {
      linkElement.style.backgroundColor="#f6f6f6";
    }
  }
}

function createInputForAp(ap) {
  if (ap.essid=="" && ap.rssi==0) return;
  var encVal="-64"; //assume wpa/wpa2
  if (ap.enc=="0") encVal="0"; //open
  if (ap.enc=="1") encVal="-32"; //wep

  var element=document.createElement("li");
  var link=document.createElement("a");
  link.className="ui-btn ui-btn-icon-right ui-icon-carat-r";
  var linkName = "link"+ap.essid;
  link.id=linkName;
  link.textContent=ap.essid;
  link.onclick = function () {
    clearSelectedItem();
    document.getElementById("essid").value = ap.essid;
    document.getElementById(linkName).style.backgroundColor="#ededed";
  };
  element.appendChild(link);
  return element;
}

function getSelectedEssid() {
  var e=document.forms.wifiform.elements;
  for (var i=0; i<e.length; i++) {
    if (e[i].type=="radio" && e[i].checked) return e[i].value;
  }
  return currAp;
}

function scanAPs() {
  xhr.open("GET", "wifiscan.cgi");
  xhr.onreadystatechange=function() {
    if (xhr.readyState==4 && xhr.status>=200 && xhr.status<300) {
      var data=JSON.parse(xhr.responseText);
      currAp=getSelectedEssid();
      if (data.result.inProgress=="0" && data.result.APs.length>1) {
        $("#aps").innerHTML="";
        $("#aps").appendChild(createDivider());
        for (var i=0; i<data.result.APs.length; i++) {
          if (data.result.APs[i].essid=="" && data.result.APs[i].rssi==0) {
            continue;
          }
          networks.push(data.result.APs[i].essid);
          $("#aps").appendChild(createInputForAp(data.result.APs[i]));
        }
        window.setTimeout(scanAPs, 20000);
      } else {
        window.setTimeout(scanAPs, 1000);
      }
    }
  }
  xhr.send();
}

window.onload=function(e) {
  scanAPs();
};
</script>
</head>
<body class="ui-mobile-viewport ui-overlay-a">
  <div data-role="page" id="pageone" data-url="pageone" tabindex="0" class="ui-page ui-page-theme-a ui-page-active" style="min-height: 459px;">
	<div data-role="header" role="banner" class="ui-header ui-bar-inherit">
		<h1 class="ui-title" role="heading" aria-level="1">JADJA</h1>
	</div>
	<div data-role="main" class="ui-content">
		<ul id="aps" data-role="listview" data-inset="true" class="ui-listview ui-listview-inset ui-corner-all ui-shadow">
      <li><a href="#" class="ui-btn ui-btn-inner ui-corner-all ui-shadow" style="text-align: center;">Scanning... Please wait...</a></li>
		</ul>
	</div>

	<div class="inputSection">
    <form name="wifiform" action="connect.cgi" method="post">
    <input type="hidden" id="essid" name="essid" value="" />
		<label for="basic">WiFi password, if applicable: </label>
		<div class="ui-input-text ui-body-inherit ui-corner-all ui-shadow-inset">
    <input type="password" name="passwd" val="%WiFiPasswd%"></div>

    <div data-role="collapsible" onclick="collapse('advanced', 'advancedIcon');" data-collapsed-icon="carat-d" data-expanded-icon="carat-u" class="ui-collapsible ui-collapsible-inset ui-corner-all ui-collapsible-themed-content ui-collapsible-collapsed">
      <h4 class="ui-collapsible-heading ui-collapsible-heading-collapsed">
        <a href="#" id="advancedIcon" class="ui-collapsible-heading-toggle ui-btn ui-btn-icon-left ui-btn-inherit ui-icon-carat-d">Advanced</a></h4>
        <div id="advanced" class="ui-collapsible-content ui-body-inherit ui-collapsible-content-collapsed">

          <label for="basic">Server: </label>
          <div class="ui-input-text ui-body-inherit ui-corner-all ui-shadow-inset">
          <input type="text" name="passwd" val=""></div>

        </div>
    </div>


    <div class="ui-input-btn ui-btn ui-corner-all ui-shadow">Connect
      <input type="submit" name="connect" value="Connect" />
    </div>

    </form>
	</div>


  </div>
</body>
</html>
