import { EditorView, basicSetup } from "codemirror";
import { python } from "@codemirror/lang-python";
import { oneDark } from "@codemirror/theme-one-dark";

let editor = new EditorView({
    extensions: [basicSetup, python(), oneDark],
    doc: "# python code here\n",
    parent: document.getElementById("editor"),
});

function log_msg(msg) {
    document.getElementById("msg").innerHTML = msg;
    $("#msg").fadeIn(2000, "linear");
    $("#msg").fadeOut(5000, "linear");
}

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

    log_msg("file opened");
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
    log_msg("file saved");
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
    log_msg("code run");
}
var run_btn = document.getElementById("run_btn");
run_btn.addEventListener("click", runCode);

// ----------------------------------------------------------------------
// terminal

$(function () {
    var code = "";
    var env = {};

    let repl_respond = function (json, term) {
        term.echo(JSON.stringify(json));
    };

    let repl_send = function (code, term) {
        fetch("/api/repl/send", {
            method: "POST",
            body: JSON.stringify({
                content: code,
            }),

            headers: {
                "Content-type": "application/json; charset=UTF-8",
            },
        })
            .then((response) => response.json())
            // .then((json) => console.log(json));
            .then((json) => repl_respond(json, term));
    };

    $("#terminal").terminal(
        [
            {
                hello: function (what) {
                    this.echo(
                        "Hello, " + what + ". Wellcome to this terminal."
                    );
                },

                cat: function (width, height) {
                    return $(
                        '<img src="https://placekitten.com/' +
                            width +
                            "/" +
                            height +
                            '">'
                    );
                },

                title: function () {
                    return fetch("https://terminal.jcubic.pl")
                        .then((r) => r.text())
                        .then(
                            (html) => html.match(/<title>([^>]+)<\/title>/)[1]
                        );
                },

                // opts like argument parsing (-a / --a)
                demo: function (...args) {
                    const options = $.terminal.parse_options(args);
                    return options;
                },

                clear: function () {
                    this.clear;
                },

                py: {
                    eval: function (arg) {},
                    exec: function (arg) {},
                    run: function (arg) {},
                    load: function (arg) {},
                    save: function (arg) {},
                },

                name: function (name) {
                    this.push(
                        function (last_name) {
                            if (last_name) {
                                this.echo(
                                    "Your name is " + name + " " + last_name
                                ).pop();
                            }
                        },
                        {
                            prompt: "last name: ",
                        }
                    );
                },
            },
            function (command) {
                repl_send(command, this);
                console.log(command);
            },
        ],
        {
            keymap: {
                "CTRL-C": function (e, original) {
                    this.echo("my shortcut");
                },
                TAB: function (e, original) {
                    this.insert("    ");
                },
            },
            checkArity: false,
            completion: true,
            greetings: "Python 3.11.3\n",
            prompt: ">>> ",
        }
    );
});
$.terminal.syntax("python");
$.terminal.prism_formatters = {
    prompt: true,
    echo: true,
    animation: true, // will be supported in version >= 2.32.0
    command: true,
};

document.getElementById("default-tab").click();
