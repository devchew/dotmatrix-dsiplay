const { merge } = require("webpack-merge");
const commonConfig = require("./webpack.common");
const HtmlInlineScriptPlugin = require('html-inline-script-webpack-plugin');
const TerserPlugin = require("terser-webpack-plugin");
const { DefinePlugin } = require("webpack");

const prodConfig = {
  mode: "production",
  optimization: {
    minimize: true,
    minimizer: [
      new TerserPlugin({
        terserOptions: {
          format: {
            quote_style: 1
          }
        }
      })
    ],
  },
  plugins: [
    new DefinePlugin({
      'process.env.API': JSON.stringify("/api"),
    }),
    new HtmlInlineScriptPlugin()
  ],
};

module.exports = merge(commonConfig, prodConfig);
