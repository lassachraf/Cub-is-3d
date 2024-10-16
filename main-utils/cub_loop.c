/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cub_loop.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alassiqu <alassiqu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 16:35:57 by alassiqu          #+#    #+#             */
/*   Updated: 2024/10/15 22:33:26 by alassiqu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../cub3d_bonus.h"

float	get_current_angle(float ray_start, float ang_inc, int x)
{
	float	angle;

	angle = ray_start + (x * ang_inc);
	return (normalize_angle(angle));
}

float	*get_index(t_cub3d *cub, float curr_ang, float ray_dist)
{
	float	*ray;

	ray = malloc(2 * sizeof(float));
	if (ray == NULL)
		ft_error(cub, "Error: allocation failed.");
	ray[0] = cub->player->x + cos(curr_ang) * ray_dist;
	ray[1] = cub->player->y + sin(curr_ang) * ray_dist;
	return (ray);
}

void	cast_fov(t_cub3d *cub)
{
	int     i;
	float   ang_inc;
	float	curr_ang;
	float   ray_start;
	float	ray_dist;

	i = -1;
	ang_inc = cub->player->fov / cub->wov;
	ray_start = cub->player->rot_ang - (cub->player->fov / 2.0);
	while (++i < cub->wov)
	{
		curr_ang = get_current_angle(ray_start, ang_inc, i);
		ray_dist = calculate_ray_distance(cub, curr_ang);
		dda(cub, get_index(cub, curr_ang, ray_dist));
		// Should be done with the function "draw_and_coloring".
		int wall_height = (int)(cub->hov / ray_dist);
		int wall_top = (cub->hov - wall_height) / 2;
		int wall_bottom = wall_top + wall_height;
		for (int y = 0; y < wall_top; y++)
			my_mlx_pixel_put(cub, i, y, cub->map->colors->ceil_h);
		draw_wall_slice(cub, i, wall_height, ray_dist, curr_ang);
		for (int y = wall_bottom; y < cub->hov; y++)
			my_mlx_pixel_put(cub, i, y, cub->map->colors->floor_h);
	}
}

void draw_circle(t_cub3d *cub, int x_center, int y_center, int radius, int color)
{
	int x, y;

	for (y = -radius; y <= radius; y++)
	{
		for (x = -radius; x <= radius; x++)
		{
			if (x * x + y * y <= radius * radius)
				my_mlx_pixel_put(cub, x_center + x, y_center + y, color);
		}
	}
}

void draw_minimap(t_cub3d *cub)
{
	int map_x, map_y;
	int tile_size;
	int player_radius;
	int start_x = 20;  // Minimap position on the screen (x-offset)
	int start_y = 20;  // Minimap position on the screen (y-offset)

	// Calculate the scaling factor based on the window size and map size
	int max_map_size = cub->map->width > cub->map->height ? cub->map->width : cub->map->height;
	int max_minimap_size = cub->wov / 4 - 20;  // Limit minimap to 1/4th of window width
	tile_size = max_minimap_size / max_map_size;  // Scale tiles based on map size
	if (tile_size < 4) tile_size = 4;  // Ensure a minimum size for visibility
	player_radius = tile_size / 2;  // Scale the player's size accordingly

	// Draw a border for the minimap
	int border_thickness = 2;
	for (int y = -border_thickness; y < cub->map->height * tile_size + border_thickness; y++)
	{
		for (int x = -border_thickness; x < cub->map->width * tile_size + border_thickness; x++)
		{
			if (x < 0 || y < 0 || x >= cub->map->width * tile_size || y >= cub->map->height * tile_size)
				my_mlx_pixel_put(cub, start_x + x, start_y + y, 0x333333);  // Border color (dark grey)
		}
	}

	// Loop through the map and draw each tile
	for (map_y = 0; map_y < cub->map->height; map_y++)
	{
		for (map_x = 0; map_x < cub->map->width; map_x++)
		{
			int color = 0;

			// Only draw the tile if it's a wall, empty space, or door
			if (cub->map->map[map_y][map_x] == '1')
				color = 0x1E90FF;  // Dodger blue for walls
			else if (cub->map->map[map_y][map_x] == '0'
				|| is_it_player(cub->map->map[map_y][map_x]))
				color = 0xEEEEEE;  // Light grey for empty spaces
			else if (cub->map->map[map_y][map_x] == 'D')
				color = 0x32CD32;  // Lime green for doors
			else if (cub->map->map[map_y][map_x] == ' ')  // Empty space, leave transparent
				continue;

			// Draw the tile on the minimap
			for (int y = 0; y < tile_size; y++)
			{
				for (int x = 0; x < tile_size; x++)
				{
					my_mlx_pixel_put(cub, start_x + map_x * tile_size + x, start_y + map_y * tile_size + y, color);
				}
			}
		}
	}

	// Draw the player as a circle on the minimap
	int player_x = start_x + cub->player->x * tile_size;
	int player_y = start_y + cub->player->y * tile_size;
	draw_circle(cub, player_x, player_y, player_radius, 0xFF4500);  // Orange red for the player
}

void put_player(t_cub3d *cub)
{
	// static int		frame = 0;
	// static int		delay = 0;
	int				i, j;
	int 			x_pos, y_pos;
	unsigned int	color;

	// Return if there's no image or frame to render
	if (cub->fstp == NULL || cub->fstp->img == NULL)
		return;
	cub->fstp->img = mlx_xpm_file_to_image(cub->mlx, "animation/ply-0.xpm",
		&cub->fstp->width, &cub->fstp->height);
	cub->fstp->add = mlx_get_data_addr(cub->fstp->img, &cub->fstp->bpp, &cub->fstp->szl, &cub->fstp->end);
	// Calculate the player's position
	x_pos = (cub->wov / 2) - (cub->fstp->width / 2);
	y_pos = cub->hov - cub->fstp->height + 1;

	// Delay and frame control
	// delay++;
	// if (delay >= 10)  // Adjust '10' for animation speed
	// {
	// 	frame = (frame + 1) % 6; // Assuming 3 animation frames
	// 	load_player_frame(cub, frame);
	// 	delay = 0;
	// }

	// Render the current frame
	for (i = 0; i < cub->fstp->height; i++)
	{
		for (j = 0; j < cub->fstp->width; j++)
		{
			color = get_texture_color(cub->fstp, j, i);
			if (color != 0xFF000000) // Skip transparent pixels
				my_mlx_pixel_put(cub, x_pos + j, y_pos + i, color);
		}
	}
}




int	cub_loop(t_cub3d *cub)
{
	if (cub->img)
		mlx_destroy_image(cub->mlx, cub->img);
	cub->img = mlx_new_image(cub->mlx, cub->wov, cub->hov);
	cub->add = mlx_get_data_addr(cub->img, &cub->bpp, &cub->szl, &cub->end);
	cast_fov(cub);
	draw_minimap(cub);

	if (cub->person == 0)
		draw_gun(cub, 0);
	else if (cub->person == 1)
	{
		gun_shots(cub);
	}

	mlx_put_image_to_window(cub->mlx, cub->win, cub->img, 0, 0);
	return (0);
}
