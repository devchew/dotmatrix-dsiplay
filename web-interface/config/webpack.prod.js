const { merge } = require("webpack-merge");
const commonConfig = require("./webpack.common");
const HtmlInlineScriptPlugin = require('html-inline-script-webpack-plugin');
const CssMinimizerPlugin = require("css-minimizer-webpack-plugin");
const TerserPlugin = require("terser-webpack-plugin");
const { DefinePlugin } = require("webpack");
const HTMLInlineCSSWebpackPlugin = require("html-inline-css-webpack-plugin").default;

const prodConfig = {
  mode: "production",
  optimization: {
    minimize: true,
    minimizer: [
      new CssMinimizerPlugin(),
      new TerserPlugin({
        terserOptions: {
          format: {
            quote_style: 1,
            
          }
        }
      }),
    ],
  },
  plugins: [
    
    new DefinePlugin({
      'process.env.API': JSON.stringify("/api"),
    }),
    new HtmlInlineScriptPlugin(),
    new HTMLInlineCSSWebpackPlugin({
      styleTagFactory({ style }) {
        return `<style type='text/css'>${style}</style>`;
      },
    }),
  ],
};

module.exports = merge(commonConfig, prodConfig);
