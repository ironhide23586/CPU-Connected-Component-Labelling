#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;

struct ccl_tracker_elem {
  int connected_points;
  std::pair<int, int> start_point;
  int next_nz;
  int prev_nz;
};

void populate_ccl_tracker(std::vector<std::vector<char>> &board,
                          std::vector<std::vector<int>> &board_tracker,
                          char foreground,
                          std::vector<ccl_tracker_elem> &ccl_tracker) {
  int board_h = board.size(), board_w = board[0].size();
  int segmented_board_h = std::ceilf((float)board_h / 2);
  int segmented_board_w = std::ceilf((float)board_w / 2);
  int init_labels = segmented_board_w / segmented_board_w;
  int curr_label, prev_nz, corner_r, corner_c;
  int curr_local_nz = -1, prev_local_nz = -1;
  bool fg_found = false;
  for (int i = 0; i < segmented_board_h; i++) {
    for (int j = 0; j < segmented_board_w; j++) {
      corner_r = i * 2;
      corner_c = j * 2;
      curr_label = i * segmented_board_w + j;
      for (int local_r = corner_r; 
           local_r <= corner_r + 1 && local_r < board_h; local_r++) {
        for (int local_c = corner_c; 
             local_c <= corner_c + 1 && local_c < board_w; local_c++) {
          if (board[local_r][local_c] == foreground) {
            board_tracker[local_r][local_c] = curr_label;
            ccl_tracker[curr_label].connected_points++;
            ccl_tracker[curr_label].start_point
              = std::make_pair(local_r, local_c);
            fg_found = true;
          }
        }
      }
      if (fg_found) {
        if (curr_local_nz != -1) {
          ccl_tracker[curr_label].prev_nz = prev_nz;
          ccl_tracker[prev_nz].next_nz = curr_label;
        }
        curr_local_nz = curr_label;
        prev_nz = curr_label;
        fg_found = false;
      }
    }
  }
}

void print_mat(std::vector<std::vector<char>> &mat) {
  for (int i = 0; i < mat.size(); i++) {
    for (int j = 0; j < mat[i].size(); j++) {
      std::cout << mat[i][j] << ",\t";
    }
    std::cout << std::endl;
  }
  std::cout << "-------------------------" << std::endl;
}

void print_mat(std::vector<std::vector<int>> &mat) {
  for (int i = 0; i < mat.size(); i++) {
    for (int j = 0; j < mat[i].size(); j++) {
      std::cout << mat[i][j] << ",\t";
    }
    std::cout << std::endl;
  }
  std::cout << "-------------------------" << std::endl;
}

//Assumes 8-way connectivity
void consume_region_worker(std::vector<std::vector<int>> &board_tracker,
                           std::pair<int, int> center_point, int label,
                           std::vector<ccl_tracker_elem> &ccl_tracker) {
  if (((center_point.first < 0 || center_point.first >= board_tracker.size())
       || (center_point.second < 0 || center_point.second 
           >= board_tracker[0].size()))
      || (board_tracker[center_point.first][center_point.second] == -1)) {
    return;
  }
  board_tracker[center_point.first][center_point.second] = -1;
  int segmented_tile_lin_idx = center_point.first / 2 
    * std::ceilf((float)board_tracker[0].size() / 2)
    + center_point.second / 2;
  ccl_tracker[segmented_tile_lin_idx].connected_points--;
  ccl_tracker[label].connected_points++;
  if (ccl_tracker[segmented_tile_lin_idx].connected_points <= 0) {
    if (ccl_tracker[segmented_tile_lin_idx].prev_nz >= 0)
      ccl_tracker[ccl_tracker[segmented_tile_lin_idx].prev_nz].next_nz
      = ccl_tracker[segmented_tile_lin_idx].next_nz;
    if (ccl_tracker[segmented_tile_lin_idx].next_nz >= 0)
      ccl_tracker[ccl_tracker[segmented_tile_lin_idx].next_nz].prev_nz
      = ccl_tracker[segmented_tile_lin_idx].prev_nz;
    ccl_tracker[segmented_tile_lin_idx].next_nz = -1;
    ccl_tracker[segmented_tile_lin_idx].prev_nz = -1;
  }
  consume_region_worker(board_tracker, std::make_pair(center_point.first - 1,
                                                      center_point.second - 1),
                        label, ccl_tracker);
  consume_region_worker(board_tracker, std::make_pair(center_point.first - 1,
                                                      center_point.second),
                        label, ccl_tracker);
  consume_region_worker(board_tracker, std::make_pair(center_point.first - 1,
                                                      center_point.second + 1),
                        label, ccl_tracker);
  consume_region_worker(board_tracker, std::make_pair(center_point.first,
                                                      center_point.second - 1),
                        label, ccl_tracker);
  consume_region_worker(board_tracker, std::make_pair(center_point.first,
                                                      center_point.second + 1),
                        label, ccl_tracker);
  consume_region_worker(board_tracker, std::make_pair(center_point.first + 1, 
                                                      center_point.second - 1),
                        label, ccl_tracker);
  consume_region_worker(board_tracker, std::make_pair(center_point.first + 1, 
                                                      center_point.second),
                        label, ccl_tracker);
  consume_region_worker(board_tracker, std::make_pair(center_point.first + 1, 
                                                      center_point.second + 1),
                        label, ccl_tracker);
}

void consume_region(std::vector<std::vector<int>> &board_tracker,
                    std::pair<int, int> center_point,
                    std::vector<ccl_tracker_elem> &ccl_tracker) {
  int center_segmented_tile_lin_idx = center_point.first / 2
    * std::ceilf((float)board_tracker[0].size() / 2)
    + center_point.second / 2;
  consume_region_worker(board_tracker, center_point,
                        center_segmented_tile_lin_idx, ccl_tracker);
}

int process_ccl_board_trackers(std::vector<std::vector<int>> &board_tracker,
                                std::vector<ccl_tracker_elem> &ccl_tracker) {
  int connected_labels = 0, curr_seg_label = 0;
  while (curr_seg_label >= 0) {
    if (ccl_tracker[curr_seg_label].connected_points <= 0)
      break;
    consume_region(board_tracker, ccl_tracker[curr_seg_label].start_point,
                   ccl_tracker);
    curr_seg_label = ccl_tracker[curr_seg_label].next_nz;
    connected_labels++;
  }
  return connected_labels;
}

int find_num_components(std::vector<std::vector<char>> &board, 
                        char foreground) {
  int board_h = board.size(), board_w = board[0].size();
  int segmented_board_h = std::ceilf((float)board_h / 2);
  int segmented_board_w = std::ceilf((float)board_w / 2);
  int init_labels = segmented_board_h * segmented_board_w;
  std::vector<ccl_tracker_elem> ccl_tracker(init_labels);
  std::vector<std::vector<int>> board_tracker(board_h, 
                                              std::vector<int>(board_w, -1));
  for (int i = 0; i < init_labels; i++) {
    ccl_tracker[i].connected_points = 0;
    ccl_tracker[i].next_nz = -1;
    ccl_tracker[i].prev_nz = -1;
  }
  populate_ccl_tracker(board, board_tracker, foreground, ccl_tracker);
  return process_ccl_board_trackers(board_tracker, ccl_tracker);
}

std::vector<std::vector<char>> extract_board(std::vector<std::vector<char>>
                                             &board, int x1, int y1,
                                             int x2, int y2) {
  std::vector<std::vector<char>> extracted_board((x2 - x1 + 1),
                                                 std::vector<char>
                                                 (y2 - y1 + 1));
  int r1 = x1 - 1, c1 = y1 - 1;
  int r2 = x2 - 1, c2 = y2 - 1;
  for (int i = r1; i <= r2; i++) {
    for (int j = c1; j <= c2; j++) {
      extracted_board[i - r1][j - c1] = board[i][j];
    }
  }
  return extracted_board;
}

// Counts the number of connected regions with input label assuming 
// 8-way connectivity
int main() {
  std::vector<std::vector<char>> board(5, std::vector<char>(5));
  board[0] = { 'B', 'W', 'B', 'B', 'W' };
  board[1] = { 'B', 'W', 'W', 'B', 'B' };
  board[2] = { 'W', 'B', 'W', 'W', 'W' };
  board[3] = { 'B', 'B', 'B', 'B', 'W' };
  board[4] = { 'W', 'W', 'B', 'B', 'W' };

  std::cout << "Matrix -" << std::endl;
  print_mat(board);
  int B_Count = find_num_components(board, 'B');
  int W_Count = find_num_components(board, 'W');

  std::cout << "Number of 'B' regions = " << B_Count << std::endl;
  std::cout << "Number of 'W' regions = " << W_Count << std::endl;

  getchar();
  return 0;
}
