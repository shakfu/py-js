// demo.js

window.max.bindInlet('say_hello', function() {
	const greeting = "hello world!!!";
	alert(`we say: ${greeting}`);
});

window.max.bindInlet('out', function (a) {
	// output a string
	// window.max.outlet(a);

	// output a list
	window.max.outlet(a, 1, 2);

	// output contents of array with prepended 'foo' message
	var ar = [1, 2, 3, 4];
	// window.max.outlet.apply(window.max, ['foo'].concat(ar));
});
