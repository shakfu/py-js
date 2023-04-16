import { EditorView, basicSetup } from "codemirror";
import { python } from "@codemirror/lang-python";
import { oneDark } from "@codemirror/theme-one-dark";

// import "xterm/css/xterm.css";
import { Terminal } from "xterm";
import { Readline } from "xterm-readline";
import { FitAddon } from "xterm-addon-fit";

let editor = new EditorView({
    extensions: [basicSetup, python(), oneDark],
    doc: "# python code here\n",
    parent: document.body,
});

function openCode() {
    var input = document.createElement("input");
    input.type = "file";

    input.onchange = (e) => {
        // getting a hold of the file reference
        var file = e.target.files[0];

        // setting up the reader
        var reader = new FileReader();
        reader.readAsText(file, "UTF-8");

        // here we tell the reader what to do when it's done reading...
        reader.onload = (readerEvent) => {
            var content = readerEvent.target.result; // this is the content!
            // console.log(content);
            let transaction = editor.state.update({
                changes: {
                    from: 0,
                    to: editor.state.doc.length,
                    insert: content,
                },
            });
            // console.log(transaction.state.doc.toString()); // "0123"
            // At this point the view still shows the old state.
            editor.dispatch(transaction);
        };
    };

    input.click();

    document.getElementById("msg").innerHTML = "file opened";
}
var open_btn = document.getElementById("open_btn");
open_btn.addEventListener("click", openCode);

function saveCode() {
    fetch("/api/code/save", {
        method: "POST",
        body: JSON.stringify({
            file_id: 1,
            content: editor.state.doc.toString(),
        }),

        headers: {
            "Content-type": "application/json; charset=UTF-8",
        },
    });
    // .then((response) => response.json())
    // .then((json) => console.log(json));

    console.log(editor.state.doc.toString());
    document.getElementById("msg").innerHTML = "file saved";
}
var save_btn = document.getElementById("save_btn");
save_btn.addEventListener("click", saveCode);

function runCode() {
    fetch("/api/code/run", {
        method: "POST",
        body: JSON.stringify({
            file_id: 1,
            content: editor.state.doc.toString(),
        }),

        headers: {
            "Content-type": "application/json; charset=UTF-8",
        },
    });
    // .then((response) => response.json())
    // .then((json) => console.log(json));

    console.log(editor.state.doc.toString());
    document.getElementById("msg").innerHTML = "code run";
}
var run_btn = document.getElementById("run_btn");
run_btn.addEventListener("click", runCode);

// ----------------------------------------------------------------------
// terminal

const term = new Terminal({
    theme: {
        background: "#191A19",
        foreground: "#F5F2E7",
    },
    cursorBlink: true,
    cursorStyle: "block",
});

// term.write("Python 3.11.2 (web-editor)!\n");

const fitAddon = new FitAddon();
const readlineAddon = new Readline();

term.loadAddon(fitAddon);
term.loadAddon(readlineAddon);
term.open(document.getElementById("terminal"));
fitAddon.fit();
term.focus();

readlineAddon.setCheckHandler((text) => {
    let trimmedText = text.trimEnd();
    // if (trimmedText.endsWith("&&")) {
    if (trimmedText.endsWith("\\")) {
        return false;
    }
    return true;
});

function readLine() {
    readlineAddon.read(">>> ").then(sendReplEntry);
}

// function processLine(text) {
// readlineAddon.println("");
// readlineAddon.println("you entered: \n" + text.replace("\\\\", ""));
// setTimeout(readLine);
// }

function respondReplEntry(json) {
    readlineAddon.println(JSON.stringify(json));
    setTimeout(readLine);
}

function sendReplEntry(text) {
    // readlineAddon.println("");
    // readlineAddon.println("you entered: \n" + text.replace("\\\\", ""));
    // setTimeout(readLine);
    fetch("/api/repl/send", {
        method: "POST",
        body: JSON.stringify({
            content: text,
        }),

        headers: {
            "Content-type": "application/json; charset=UTF-8",
        },
    })
        .then((response) => response.json())
        // .then((json) => console.log(json));
        .then((json) => respondReplEntry(json));
}

readLine();
