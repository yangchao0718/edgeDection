# include"outXls.h"

//template< class T>
void outXls(string str, Mat im, string dataType)
{
	//establised an output FILE IO Object
	ofstream outfile(str);
	auto a = typeid(*im.data).name();

	for (int i = 0; i < im.rows; i++)
	{
		for (int j = 0; j < im.cols; j++)
		{
			if (dataType == "float")
			{
				outfile << im.at<float>(i, j) << "\t";
			}
			else if (dataType == "int")
			{
				outfile << im.at<int>(i, j) << "\t";
			}
			else 
			{
				cout << "input dataType Error!\n";
			}

		}
		//change line
		outfile << "\n";
	}

	//close file io 
	outfile.close();

}

