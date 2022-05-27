// CREDIT: original script provided by TO_THE_SUN in a Max Forum discussion about Python and Max
// see: https://cycling74.com/forums/python-in-max-1/replies/1#reply-60d6300b43a79a06e08ef609


const path = require('path');
const {PythonShell} = require('python-shell');
const maxAPI = require("max-api");

// This will be printed directly to the Max console
//Max.post(`Loaded the ${path.basename(__filename)} script`);


PythonShell.runString('x=1+1;print(x)', null, function (err) {
  if (err) throw err;
  console.log('python execution finished');
});

let options = {
  mode: 'text',
  pythonPath: '/usr/local/bin/python3',
  pythonOptions: ['-u'], // get print results in real-time
  //scriptPath: 'path/to/my/scripts',
  //args: ['value1', 'value2', 'value3']
};


// let pyshell = new PythonShell('hello.py');

let pyshell = new PythonShell('test_pynode.py');

//import {PythonShell} from 'python-shell';
// let pyshell = new PythonShell('pipeline.py');
// sends a message to the Python script via stdin
//pyshell.send('hello');

pyshell.on('message', function (message) {
  // received a message sent from the Python script (a simple "print" statement)
  //console.log(message);
  maxAPI.post(message);
	maxAPI.outlet(message);
});
pyshell.on('stderr', function (stderr) {
  // handle stderr (a line of text from stderr)
  //console.log(stderr);
	maxAPI.outlet(stderr);
});
/*
// end the input stream and allow the process to exit
pyshell.end(function (err,code,signal) {
  if (err) throw err;
  console.log('The exit code was: ' + code);
  console.log('The exit signal was: ' + signal);
  console.log('finished');
  maxAPI.post(err);
});
*/
const handlers = {
	transcribe: (symbol) => {
    	pyshell.send(symbol);
		//maxAPI.post("input: " + symbol);
	},
	name: (moniker) => {
        //maxAPI.post(moniker);
        currentFileName = moniker;
        createTxt(1, moniker);
	}
}


// Use the 'addHandler' function to register a function for a particular message
maxAPI.addHandler("bang", () => {
	maxAPI.post("Who you think you bangin'?");
});

// Use the 'outlet' function to send messages out of node.script's outlet
maxAPI.addHandler("echo", (msg) => {
	maxAPI.outlet(msg);
});

maxAPI.addHandlers(handlers);