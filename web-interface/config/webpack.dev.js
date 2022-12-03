const { merge } = require("webpack-merge");
const commonConfig = require("./webpack.common");
const { DefinePlugin } = require("webpack");

const devConfig = {
  mode: "development",
  devtool: "inline-source-map",
  devServer: {
    port: 3000,
    historyApiFallback: true,
  },
  plugins: [
    new DefinePlugin({
      'process.env.API': JSON.stringify("http://localhost:2137/api"),
    })
  ],
};

module.exports = merge(commonConfig, devConfig);
