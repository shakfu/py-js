// compare two strings for equality

inlets = 2;
outlets = 1;

var ref_string = "";

function anything()
{
	var temp = messagename.replace(/\r/g, '');

	if (inlet == 0) {
		if (temp == ref_string)
			outlet(0, 1);
		else
			outlet(0, 0);
	}
	else {
		ref_string = temp;
	}
}
