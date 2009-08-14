var DHTML = 0, DOM = 0, MS = 0, NS = 0, OP = 0;

function DHTML_init() {

	if (window.opera) {
		OP = 1;
	}
	if(document.getElementById) {
		DHTML = 1;
		DOM = 1;
	}
	if(document.all && !OP) {
		DHTML = 1;
		MS = 1;
	}
	if(document.layers && !OP) {
		DHTML = 1;
		NS = 1;
	}
}

function getElem(p1,p2,p3) {
	var Elem;
	if(DOM) {
		if(p1.toLowerCase()=="id") {
			if (typeof document.getElementById(p2) == "object")
				Elem = document.getElementById(p2);
			else Elem = void(0);
			return(Elem);
		}
		else if(p1.toLowerCase()=="name") {
			if (typeof document.getElementsByName(p2) == "object")
				Elem = document.getElementsByName(p2)[p3];
			else Elem = void(0);
			return(Elem);
		}
		else if(p1.toLowerCase()=="tagname") {
			if (typeof document.getElementsByTagName(p2) == "object" ||
				(OP && typeof document.getElementsByTagName(p2) == "function"))
				Elem = document.getElementsByTagName(p2)[p3];
			else Elem = void(0);
			return(Elem);
		}
		else return void(0);
	}
	else if(MS) {
		if(p1.toLowerCase()=="id") {
			if (typeof document.all[p2] == "object")
				Elem = document.all[p2];
			else Elem = void(0);
			return(Elem);
		}
		else if(p1.toLowerCase()=="tagname") {
			if (typeof document.all.tags(p2) == "object")
				Elem = document.all.tags(p2)[p3];
			else Elem = void(0);
			return(Elem);
		}
		else if(p1.toLowerCase()=="name") {
			if (typeof document[p2] == "object")
				Elem = document[p2];
			else Elem = void(0);
			return(Elem);
		}
		else return void(0);
	}
	else if(NS) {
		if(p1.toLowerCase()=="id" || p1.toLowerCase()=="name") {
			if (typeof document[p2] == "object")
				Elem = document[p2];
			else Elem = void(0);
			return(Elem);
		}
		else if(p1.toLowerCase()=="index") {
			if (typeof document.layers[p2] == "object")
				Elem = document.layers[p2];
			else Elem = void(0);
			return(Elem);
		}
		else return void(0);
	}
}

DHTML_init();
