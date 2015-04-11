<!DOCTYPE html>
<html class="ui-mobile">
<head>
<title>JADJA</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" href="jquery.mobile-1.4.5.min.css">
<link rel="stylesheet" href="style.css">
<script type="text/javascript" src="140medley.min.js"></script>
<script type="text/javascript">

var xhr=j();
var currAp="%currSsid%";

function createDivider() {
  var divider=document.createElement("li");
  divider.className="ui-li-divider ui-bar-inherit ui-first-child";
  divider.textContent="Please select your WiFi network:";
  return divider;
}


function createInputForAp(ap) {
  if (ap.essid=="" && ap.rssi==0) return;
  var encVal="-64"; //assume wpa/wpa2
  if (ap.enc=="0") encVal="0"; //open
  if (ap.enc=="1") encVal="-32"; //wep

  /*var div=document.createElement("div");
  div.id="apdiv";
  var rssi=document.createElement("div");
  var rssiVal=-Math.floor(ap.rssi/51)*32;
  rssi.className="icon";
  rssi.style.backgroundPosition="0px "+rssiVal+"px";
  var encrypt=document.createElement("div");
  encrypt.className="icon";
  encrypt.style.backgroundPosition="-32px "+encVal+"px";
  var input=document.createElement("input");
  input.type="radio";
  input.name="essid";
  input.value=ap.essid;
  if (currAp==ap.essid) input.checked="1";
  input.id="opt-"+ap.essid;
  var label=document.createElement("label");
  label.htmlFor="opt-"+ap.essid;
  label.textContent=ap.essid;
  div.appendChild(input);
  div.appendChild(rssi);
  div.appendChild(encrypt);
  div.appendChild(label);
  return div;*/

  /*<li><a href="#" class="ui-btn ui-btn-icon-right ui-icon-carat-r">List Item</a></li>*/

  var element=document.createElement("li");
  var link=document.createElement("a");
  link.className="ui-btn ui-btn-icon-right ui-icon-carat-r";
  link.textContent=ap.essid;
  link.onclick = function () {
    document.getElementById("essid").value = ap.essid;
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
          if (data.result.APs[i].essid=="" && data.result.APs[i].rssi==0) continue;
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

    <div class="ui-input-btn ui-btn ui-corner-all ui-shadow">Connect
      <input type="submit" name="connect" value="Connect" />
    </div>

    </form>
	</div>


  </div>
</body>
</html>
