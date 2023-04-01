import React, { useState, useRef } from "react";
import ReactDOM from "react-dom";
import Editor from "@monaco-editor/react";


function App() {
  const monacoRef = useRef(null);
  const editorRef = useRef(null);

  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  // function handleEditorChange(value, event) {
  //   console.log("here is the current model value:", value);
  // }

  function handleEditorWillMount(monaco) {
    // here is the monaco instance
    // do something before editor is mounted
    //monaco.languages.typescript.javascriptDefaults.setEagerModelSync(true);
  }

  function handleEditorDidMount(editor, monaco) {
    monacoRef.current = monaco;
    editorRef.current = editor;
  }

  function fetchHello() {
    fetch(`/api/hello`)
      .then(response => response.json())
      .then((usefulData) => {
        console.log(usefulData);
        setLoading(false);
        setData(usefulData);
      })
      .catch((e) => {
        console.error(`An error occurred: ${e}`)
      });
  }

  function aboutApp() {
    alert("nice one");
  }

  function showValue() {
    alert(editorRef.current.getValue());
  }

  return (
    <>
      <button onClick={aboutApp}>About</button>
      <button onClick={showValue}>Show value</button>
      <button onClick={fetchHello}>Hello</button>
      <Editor
        height="90vh"
        defaultLanguage="python"
        theme="vs-dark"
        defaultValue="# some comment"
        beforeMount={handleEditorWillMount}
        onMount={handleEditorDidMount}
      />
    </>
  );
}

const rootElement = document.getElementById("root");
ReactDOM.render(<App />, rootElement);

export default App;