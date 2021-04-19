import { Body, Controller, Post, UploadedFile, UseInterceptors } from '@nestjs/common';
import { FileInterceptor } from '@nestjs/platform-express';

export interface ImageReqBody {
    message: string
}

@Controller('images')
export class ImagesController {
    @Post()
    @UseInterceptors(FileInterceptor('image'))
    public postImage(@UploadedFile() file, @Body() body: ImageReqBody) {
        
    }
}
