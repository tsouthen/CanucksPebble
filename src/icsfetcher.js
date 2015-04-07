Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS Ready!');

  // Send a string to Pebble
  Pebble.sendAppMessage({ '0': 1 },
    function(e) {
      console.log('Send successful.');
    },
    function(e) {
      console.log('Send failed!');
    }
  );
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('Received message: ' + JSON.stringify(e.payload));
  getIcs('van', icsCallback);
});

function icsCallback(icalData) {
  log("icsCallback start");
	var events = parseIcs(icalData);
  log("events returned");
	var i;
  var msg = {};
	for (i=0; i < events.length; i++) {
    var currEvent = events[i];
    if (currEvent !== null) {
      log("Event " + i + ":");
      logObject(currEvent);
      var idx = i * 3;
      msg[idx+1] = getSummary(currEvent, "Vancouver");
      msg[idx+2] = Math.round(currEvent.DTSTART / 1000);
      msg[idx+3] = Math.round(currEvent.DTEND / 1000);
    }     
	}
  log("icsCallback sending app msg: " + msg);
  Pebble.sendAppMessage(msg);
  log("icsCallback end");
}

var abbreviations = {
	"Anaheim" : "ANA",
	"Arizona" : "ARZ",
	"Boston" : "BOS",
	"Buffalo" : "BUF",
	"Calgary" : "CAL",
	"Carolina" : "CAR",
	"Chicago" : "CHI",
	"Colorado" : "COL",
	"Columbus" : "CBJ",
	"Dallas" : "DAL",
	"Detroit" : "DET",
	"Edmonton" : "EDM",
	"Florida" : "FLA",
	"Los Angeles" : "LAK",
	"Minnesota" : "MIN",
	"Montreal" : "MTL",
	"NY Islanders" : "NYI",
	"NY Rangers" : "NYR",
	"Nashville" : "NSH",
	"New Jersey" : "NJD",
	"Ottawa" : "OTT",
	"Philadelphia" : "PHI",
	"Pittsburgh" : "PIT",
	"San Jose" : "SJS",
	"St. Louis" : "STL",
	"Tampa Bay" : "TBL",
	"Toronto" : "TOR",
	"Washington" : "WAS",
	"Winnipeg" : "WPG"
};

var days = [ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" ];

function pad(n, width, z) {
	z = z || '0';
	n = n + '';
	return n.length >= width ? n : new Array(width - n.length + 1).join(z) + n;
}

function getSummary(event, homeTeam) {
	var msPerDay = 24 * 60 * 60 * 1000;
	var now = new Date();	
	var newSummary;
	var startDate = event.DTSTART;
	var summary = event.SUMMARY;

	if (startDate < now) {
		newSummary = event.DESCRIPTION;
	} else {
		now.setHours(0);
		now.setMinutes(0);
		now.setSeconds(0);
		now.setMilliseconds(0);
		
		if (startDate - now < (msPerDay * 7)) {
			newSummary = "" + days[startDate.getDay()];
		} else {
			newSummary = "" + (startDate.getMonth() + 1).toString() + "/" + pad(startDate.getDate(), 2);
		}
		newSummary += " " + (startDate.getHours() % 12) + ":" + pad(startDate.getMinutes(), 2);
	}
	var teams = summary.split(" at ");
	if (teams.length == 2) {
		if (teams[0] == homeTeam)
			newSummary += " @ " + abbreviations[teams[1]];
		else
			newSummary += " vs " + abbreviations[teams[0]];
	}
	return newSummary;
}

function parseIcsDate(icalStr)  {
    // icalStr = '20110914T184000Z'             
    var year = icalStr.substr(0,4);
    var month = parseInt(icalStr.substr(4,2),10)-1;
    var day = icalStr.substr(6,2);
    var hour = icalStr.substr(9,2);
    var min = icalStr.substr(11,2);
    var sec = icalStr.substr(13,2);

    var oDate = new Date();

    oDate.setUTCFullYear(year);
    oDate.setUTCMonth(month);
    oDate.setUTCDate(day);
    oDate.setUTCHours(hour);
    oDate.setUTCMinutes(min);
    oDate.setUTCSeconds(sec);
    return oDate;
}

function getIcs(team, callback) {
	var req = new XMLHttpRequest();
    req.open('GET', 'http://sports.yahoo.com/nhl/teams/' + team + '/ical.ics', true);
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
			if (req.status == 200) {
				callback(req.responseText);
			} else { 
				console.log('Error getting ical'); 
			}
		}
	};
	req.send(null);
}

function getFields(linesObj) {
	var lines = linesObj.lines;
	var linesLength = linesObj.linesLength;
	var fields = {};
	var stop = false;
	var i;
	
	for (i = linesObj.currLine; !stop && i < linesLength; i++) {
		var line = lines[i];
		var splitAt = line.indexOf(":");
		if (splitAt < 0)
      continue;
		
		var currentKey = line.substr(0, splitAt);
		var currentValue = line.substr(splitAt + 1);
		
		switch (currentKey) {
			case "DTSTART":
			case "DTEND":
				fields[currentKey] = parseIcsDate(currentValue);
				break;

			case "SUMMARY":
				fields[currentKey] = currentValue;
				break;

			case "DESCRIPTION":
				fields[currentKey] = currentValue;
				break;

			case "END":
				stop = true;
				linesObj.currLine = i;
				break;
		}
	}
	return fields;
}

function parseIcs(ics) {
	log("Parsing ical data");
	var now = new Date();
	var lines = ics.split(/\r\n|\n|\r/);
	log("Parsing " + lines.length + " lines");

	var found = false;
	var i, linesLength, lastStart;
	
	for (i = 0, linesLength = lines.length; i < linesLength; i++) {
		var line = lines[i];
		var splitAt = line.indexOf(":");
		if (splitAt < 0)
			continue;
		
		var currentKey = line.substr(0, splitAt);

		if (currentKey == "DTSTART" && !found) {
			var currentValue = line.substr(splitAt + 1);
			var currDate = parseIcsDate(currentValue);
			if (currDate > now) {
				//log("Found next event: " + currDate);
				
				//get last event
				var linesObj = { "lines": lines, "linesLength": linesLength, "currLine": lastStart };
				var lastEvent = {};
				if (lastStart !== 0) {
					lastEvent = getFields(linesObj);
				} else {
          lastEvent = {"SUMMARY" : "no previous game"};
        }
				//get next event
				linesObj.currLine = i;
				var nextEvent = getFields(linesObj, currDate);				
				return [lastEvent, nextEvent];
			} else {
				lastStart = i;
			}
		}
	}
  if (lastStart !== 0) {
    return [getFields({ "lines": lines, "linesLength": linesLength, "currLine": lastStart }), {"SUMMARY" : "no next game"}];
  }
	return [{"SUMMARY" : "no previous game"}, {"SUMMARY" : "no next game"}];
}

function logObject(objToLog) {
	for (var key in objToLog) {
		log(key + ": " + objToLog[key]);
	}
}

function log(msg) {
	console.log(msg);
}
