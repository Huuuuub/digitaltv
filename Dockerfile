FROM node

RUN mkdir /service

WORKDIR /service

COPY .babelrc .
COPY .eslintrc.json .
COPY tools.js .
COPY package.json .

COPY tools .
COPY tools ./tools
COPY streams ./streams

RUN npm install

EXPOSE 3000

CMD ["bash"]