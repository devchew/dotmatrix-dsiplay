const { merge } = require("webpack-merge");
const commonConfig = require("./webpack.common");
const CssMinimizerPlugin = require("css-minimizer-webpack-plugin");
const TerserPlugin = require("terser-webpack-plugin");
const { DefinePlugin } = require("webpack");

const prodConfig = {
  mode: "production",
  optimization: {
    minimize: true,
    minimizer: [
      new CssMinimizerPlugin(),
      new TerserPlugin(),
    ],
  },
  plugins: [
    
    new DefinePlugin({
      'process.env.API': JSON.stringify("/api"),
    }),

  ],
};

module.exports = merge(commonConfig, prodConfig);
