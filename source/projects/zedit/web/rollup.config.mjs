import {nodeResolve} from "@rollup/plugin-node-resolve"
import commonjs from '@rollup/plugin-commonjs';
import css from "rollup-plugin-import-css";

export default {
  input: "./terminal.mjs",
  output: {
    file: "./public/js/terminal.bundle.js",
    format: "iife"
  },
  plugins: [
    nodeResolve(), 
    css(), 
    commonjs(),
  ]
}

