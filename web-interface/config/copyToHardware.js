const fs = require("fs");
const path = require("path");

const transform = (line) => line.replaceAll("\\", "\\\\").replaceAll('"', '\\"');

const config = {
  input: path.resolve(__dirname, "..", "dist", "index.html"),
  output: path.resolve(__dirname, "..", "..", "hardware", "http.ino"),
};

const fileContent = fs.readFileSync(config.input).toString();

const escaped = transform(fileContent)

const newFile = `void handleHome(){
    server.send(200, "text/html", "${escaped}");
}`;

fs.writeFileSync(config.output, newFile);
console.log('done')