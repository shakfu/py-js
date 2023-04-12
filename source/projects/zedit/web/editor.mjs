import { EditorView, basicSetup } from "codemirror";
import { keymap, highlightActiveLine } from "@codemirror/view";
import { defaultKeymap } from "@codemirror/commands";

import { python } from "@codemirror/lang-python";
import { oneDark } from "@codemirror/theme-one-dark";

let editor = new EditorView({
    extensions: [basicSetup, python(), oneDark, highlightActiveLine()],
    doc: "# python code here",
    parent: document.body,
});

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
    document.getElementById("msg").innerHTML = "code saved";
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
