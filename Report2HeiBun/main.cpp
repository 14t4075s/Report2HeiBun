#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include <ctime>
#include <boost/asio.hpp>
#include <utility>
#include "ppm.h"
#include <cstring>
#include <cstdlib>

#define MAX_LENGTH  2764855

using boost::asio::ip::tcp;

void processThread(int part);
void processNet();


//Split "mem" into "parts", e.g. if mem = 10 and parts = 4 you will have: 0,2,4,6,10
//if possible the function will split mem into equal chuncks, if not
//the last chunck will be slightly larger


std::vector<int> bounds(int parts, int mem) {
	std::vector<int>bnd;
	int delta = mem / parts;
	int reminder = mem % parts;
	int N1 = 0, N2 = 0;
	bnd.push_back(N1);
	for (int i = 0; i < parts; ++i) {
		N2 = N1 + delta;
		if (i == parts - 1)
			N2 += reminder;
		bnd.push_back(N2);
		N1 = N2;
	}
	return bnd;
}

//Test if a given position (ii,jj) is "inside" the limits 0..nr_lines and 0..nr_columns

bool border(int ii, int jj, int nr_lines, int nr_columns) {
	if (ii >= 0 && ii < nr_lines && jj >= 0 && jj < nr_columns)
		return true;
	else
		return false;
}

//Blur the pixel at (i,j) using information from the neighbour pixels

void process(ppm &image, ppm &image2, int i, int j) {
	int ii, jj, nr_lines, nr_columns, indx;
	unsigned int r, g, b;
	float r_sum, g_sum, b_sum;
	//Filter used for bluring an image
	float filter[] = {
		0.10179640718562874, 0.11377245508982035, 0.10179640718562874,
		0.11377245508982035, 0.1377245508982036, 0.11377245508982035,
		0.10179640718562874, 0.11377245508982035, 0.10179640718562874
	};

	nr_lines = image.height;
	nr_columns = image.width;

	//Apply the filter:
	r_sum = 0;
	g_sum = 0;
	b_sum = 0;

	//check North-West
	ii = i - 1;
	jj = j - 1;
	if (border(ii, jj, nr_lines, nr_columns)) {
		indx = ii * image.width + jj;

		r = (unsigned int)image.r[indx];
		g = (unsigned int)image.g[indx];
		b = (unsigned int)image.b[indx];

		r_sum += r * filter[0];
		g_sum += g * filter[0];
		b_sum += b * filter[0];
	}

	//check North
	ii = i - 1;
	jj = j;
	if (border(ii, jj, nr_lines, nr_columns)) {
		indx = ii * image.width + jj;

		r = (unsigned int)image.r[indx];
		g = (unsigned int)image.g[indx];
		b = (unsigned int)image.b[indx];

		r_sum += r * filter[1];
		g_sum += g * filter[1];
		b_sum += b * filter[1];
	}

	//check North-East
	ii = i - 1;
	jj = j + 1;
	if (border(ii, jj, nr_lines, nr_columns)) {
		indx = ii * image.width + jj;

		r = (unsigned int)image.r[indx];
		g = (unsigned int)image.g[indx];
		b = (unsigned int)image.b[indx];

		r_sum += r * filter[2];
		g_sum += g * filter[2];
		b_sum += b * filter[2];
	}

	//check West
	ii = i;
	jj = j - 1;
	if (border(ii, jj, nr_lines, nr_columns)) {
		indx = ii * image.width + jj;

		r = (unsigned int)image.r[indx];
		g = (unsigned int)image.g[indx];
		b = (unsigned int)image.b[indx];

		r_sum += r * filter[3];
		g_sum += g * filter[3];
		b_sum += b * filter[3];
	}

	//center
	ii = i;
	jj = j;
	indx = ii * image.width + jj;

	r = (unsigned int)image.r[indx];
	g = (unsigned int)image.g[indx];
	b = (unsigned int)image.b[indx];

	r_sum += r * filter[4];
	g_sum += g * filter[4];
	b_sum += b * filter[4];


	//check East
	ii = i;
	jj = j + 1;
	if (border(ii, jj, nr_lines, nr_columns)) {
		indx = ii * image.width + jj;

		r = (unsigned int)image.r[indx];
		g = (unsigned int)image.g[indx];
		b = (unsigned int)image.b[indx];

		r_sum += r * filter[5];
		g_sum += g * filter[5];
		b_sum += b * filter[5];
	}
	//check South-West
	ii = i + 1;
	jj = j - 1;
	if (border(ii, jj, nr_lines, nr_columns)) {
		indx = ii * image.width + jj;

		r = (unsigned int)image.r[indx];
		g = (unsigned int)image.g[indx];
		b = (unsigned int)image.b[indx];

		r_sum += r * filter[6];
		g_sum += g * filter[6];
		b_sum += b * filter[6];
	}
	//check South
	ii = i + 1;
	jj = j;
	if (border(ii, jj, nr_lines, nr_columns)) {
		indx = ii * image.width + jj;

		r = (unsigned int)image.r[indx];
		g = (unsigned int)image.g[indx];
		b = (unsigned int)image.b[indx];

		r_sum += r * filter[7];
		g_sum += g * filter[7];
		b_sum += b * filter[7];
	}
	//check South-East
	ii = i + 1;
	jj = j + 1;
	if (border(ii, jj, nr_lines, nr_columns)) {
		indx = ii * image.width + jj;

		r = (unsigned int)image.r[indx];
		g = (unsigned int)image.g[indx];
		b = (unsigned int)image.b[indx];

		r_sum += r * filter[8];
		g_sum += g * filter[8];
		b_sum += b * filter[8];
	}

	//Save the modifed pixel value in image2
	indx = i * image.width + j;
	image2.r[indx] = (unsigned char)r_sum;
	image2.g[indx] = (unsigned char)g_sum;
	image2.b[indx] = (unsigned char)b_sum;
}

//Blur a chunck of an image

void tst(ppm &image, ppm &image2, int left, int right) {
	for (int i = left; i < right; ++i) {
		int ii = i / image.width;
		int jj = i - ii * image.width;
		process(image, image2, ii, jj);
	}
}

int main() {
	int prog(0);
	int num(0);
	std::cout << "Enter Program type:		0:Thread		1:Network " << std::endl;
	std::cin >> prog;
	if (prog == 0)
	{
		std::cout << "Enter Number of Thread: " << std::endl;
		std::cin >> num;
	}
	switch (prog)
	{
	case 0:
		processThread(num);
		break;
	case 1:
		processNet();
		break;
	default:
		break;
	}
	std::cin.get();
	return 0;
}
//process all threads
void processThread(int part)
{
	std::string fname = std::string("hd2.ppm");

	ppm image(fname);
	ppm image2(image.width, image.height);

	//Number of threads to use (the image will be divided between threads)
	int parts = part;

	std::vector<int>bnd = bounds(parts, image.size);

	//std::vector<std::thread> tt(parts-1);
	std::vector<std::thread> tt;
	time_t start, end;
	time(&start);
	if (parts != 1)
	{

		//Lauch parts-1 threads
		for (int i = 0; i < parts - 1; ++i) {
			tt.push_back(std::thread(tst, std::ref(image), std::ref(image2), bnd[i], bnd[i + 1]));
			//tt[i] = std::thread(tst, std::ref(image), std::ref(image2), bnd[i], bnd[i + 1]);
		}

		//Use the main thread to do part of the work !!!
		for (int i = parts - 1; i < parts; ++i) {
			tst(image, image2, bnd[i], bnd[i + 1]);
		}

		//Join parts-1 threads
		for (auto &e : tt) {
			e.join();
		}
		time(&end);
		std::cout << difftime(end, start) << " seconds" << std::endl;
		image2.write("test.ppm");
	}
	else
	{
		tst(image, image2, bnd[0], bnd[1]);
		time(&end);
		std::cout << difftime(end, start) << " seconds" << std::endl;
		//Save the result
		image2.write("test.ppm");
	}
}
//process allocated chunk of data
void server(boost::asio::io_service& io_service, unsigned short port)
{
	tcp::resolver resolver(io_service);
	tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));
	tcp::acceptor acceptor2(io_service, tcp::endpoint(tcp::v4(), port+1));
	for ( ;  ;)
	{
		tcp::socket socket(io_service);
		tcp::socket socket2(io_service);
		boost::system::error_code errcode;
		boost::asio::streambuf recv_streambuffer;
		boost::asio::streambuf send_streambuffer;
		std::istream istream(&recv_streambuffer);
		std::ostream ostream(&send_streambuffer);
		std::ofstream view("view.ppm", std::ios::out | std::ios::binary);
		std::stringstream sst;
		acceptor.accept(socket);

		while (boost::asio::read(socket, recv_streambuffer, boost::asio::transfer_at_least(1), errcode))
		{
			sst << &recv_streambuffer;
		}
		if (errcode == boost::asio::error::eof)
			std::cout << "end of file\n";
		else if (errcode)
			std::cerr << "Error";
		std::cout << "Data received" << std::endl;
		ppm image(sst.str(), false);
		ppm image2(image.width, image.height);
		tst(image, image2, 0, image.size);
		//image.write("testTCPServer.ppm");
		ostream << image2.getString();
		std::cout << "process finished" << std::endl;
		acceptor2.accept(socket2);
		boost::asio::write(socket2, send_streambuffer);
		std::cout << "Data sent" << std::endl;
	}

}
//client side operations
//split data between servers
void client(boost::asio::io_service& io_service, unsigned short port)
{
	int servNum(0);
	int bckWidth(0);
	int bckHeight(0);
	std::string servName;
	std::vector<std::string> servers;
	std::cout << "Number of server : ";
	std::cin >> servNum;
	for (size_t i = 0; i < servNum; i++)
	{
		std::cout << "Enter server " << i + 1 << " ip adress: ";
		std::cin >> servName;
		servers.push_back(servName);
	}
	std::vector<std::string> data;
	std::string fname = std::string("hd.ppm");
	ppm image(fname);
	std::ifstream inp(fname.c_str(), std::ios::in | std::ios::binary);
	if (inp.is_open())
	{
		std::string line;
		std::getline(inp, line);
		if (line != "P6") {
			std::cout << "Error. Unrecognized file format." << std::endl;
			return;
		}
		std::getline(inp, line);
		while (line[0] == '#') {
			std::getline(inp, line);
		}
		std::stringstream dimensions(line);
		int width(0);
		int height(0);
		int width_rest(0);
		int height_rest(0);

		try {
			dimensions >> width;
			dimensions >> height;
			bckWidth = width;
			bckHeight = height;
			width_rest = width % servNum;
			width /= servNum;
			height_rest = height % servNum;
			height /= servNum;
		}
		catch (std::exception &e) {
			std::cout << "Header file format error. " << e.what() << std::endl;
			return;
		}

		std::getline(inp, line);
		std::stringstream max_val(line);
		int max_col_val(0);
		try {
			max_val >> max_col_val;
		}
		catch (std::exception &e) {
			std::cout << "Header file format error. " << e.what() << std::endl;
			return;
		}
		for (size_t i = 0; i < servNum; i++)
		{
			std::stringstream ss;
			int size(0);
			if (i == servNum - 1)
			{
				width += width_rest;
				height += height_rest;
			}
			size = width * height;
			ss << "P6\n";
			ss << width << " " << height << "\n";
			ss << max_col_val << "\n";
			char aux;
			for (size_t j = 0; j < size; j++)
			{
				inp.read(&aux, 1);
				ss.write(&aux, 1);
				inp.read(&aux, 1);
				ss.write(&aux, 1);
				inp.read(&aux, 1);
				ss.write(&aux, 1);
			}
			data.push_back(ss.str());
		}
	}

	try
	{
		time_t start, end;
		time(&start);
		for (size_t i = 0; i < servers.size(); i++)
		{
			tcp::socket socket(io_service);
			tcp::resolver resolver(io_service);
			boost::system::error_code errcode;
			boost::asio::connect(socket, resolver.resolve({ servers[i], "1335" }));
			std::cout << "connected to server " << i << std::endl;
			boost::asio::streambuf request;
			std::ostream req_stream(&request);
			req_stream << data[i];
			//boost::asio::write(socket, boost::asio::buffer(data[0].c_str(), data[0].size()));
			boost::asio::write(socket, request);
			std::cout << "data sent to server " << i + 1 << std::endl;
			socket.shutdown(boost::asio::socket_base::shutdown_both);
			socket.close();
		}
		std::stringstream sst;
		sst << "P6\n";
		sst << image.width << " " << image.height << "\n";
		sst << image.max_col_val << "\n";
		for (size_t i = 0; i < servers.size(); i++)
		{
			tcp::socket socket(io_service);
			tcp::resolver resolver(io_service);
			boost::system::error_code errcode;
			boost::asio::connect(socket, resolver.resolve({ servers[i], "1336" }));
			boost::asio::streambuf response;
			std::istream res_stream(&response);
			//boost::asio::write(socket, boost::asio::buffer(data[0].c_str(), data[0].size()));
			while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), errcode))
			{
				//view << &recv_streambuffer;
				sst << &response;
			}
			std::cout << "data received from server " << i + 1 << std::endl;
			socket.shutdown(boost::asio::socket_base::shutdown_both);
			socket.close();
		}
		ppm image2(sst.str(), false);
		time(&end);
		std::cout << difftime(end, start) << " seconds" << std::endl;
		image2.write("testTCPClient.ppm");
		std::cout << "image processing finished"<< std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		std::cin.get();
	}
}

//handling network process
void processNet()
{

	int choice(0);
	std::cout << "Server(0) Or Client(1) : " << std::endl;
	std::cin >> choice;
	if (choice == 0)
	{
		boost::asio::io_service io_service;
		server(io_service, 1335);
	}
	else
	{
		boost::asio::io_service io_service;
		client(io_service, 1335);

	}
}
